#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "Serial.h"
#include "Timer.h"
#include <math.h>

// 三维坐标结构体
typedef struct {
    float x;
    float y;
    float z;
} Point3D;

// 二维坐标结构体
typedef struct {
    int x;
    int y;
} Point2D;

float Pitch, Roll, Yaw;

// 三维坐标系顶点定义（原点和三轴端点）
Point3D origin = {0, 0, 0};
Point3D x_axis = {25, 0, 0};   // X轴长度（适配128x64屏幕）
Point3D y_axis = {0, 25, 0};   // Y轴长度
Point3D z_axis = {0, 0, 25};   // Z轴长度

// 相机 / 视角参数
float cam_yaw = -30.0f;    // 相机绕Z轴旋转角（度），用于视角调整
float cam_pitch = 20.0f;   // 相机俯仰角（度）
float cam_distance = 80.0f; // 用于透视效果的相机距离/偏移
int cam_mode = 1; // 0: 固定, 1: 自动环绕, 2: 跟随传感器

// 弧度转换宏
#define RAD(x) (x * 3.1415926f / 180.0f)

// 屏幕中心坐标（128x64 OLED）
#define CENTER_X 64
#define CENTER_Y 32
// 透视投影缩放因子
#define PROJECTION_SCALE 80

/**
 * @brief 欧拉角旋转3D点
 * @param p 原始3D点
 * @param roll 横滚角（度）
 * @param pitch 俯仰角（度）
 * @param yaw 偏航角（度）
 * @return 旋转后的3D点
 */
Point3D rotate_point(Point3D p, float roll, float pitch, float yaw) {
    float cr = cos(RAD(roll));
    float sr = sin(RAD(roll));
    float cp = cos(RAD(pitch));
    float sp = sin(RAD(pitch));
    float cy = cos(RAD(yaw));
    float sy = sin(RAD(yaw));
    
    Point3D res;
    // Z-Y-X顺序旋转矩阵计算
    res.x = p.x * (cy * cp) + p.y * (cy * sp * sr - sy * cr) + p.z * (cy * sp * cr + sy * sr);
    res.y = p.x * (sy * cp) + p.y * (sy * sp * sr + cy * cr) + p.z * (sy * sp * cr - cy * sr);
    res.z = p.x * (-sp) + p.y * (cp * sr) + p.z * (cp * cr);
    return res;
}

/**
 * @brief 将世界坐标根据相机参数变换到相机空间
 */
Point3D apply_camera_transform(Point3D p) {
    // 先做相机的俯仰（绕 X 轴）
    float cp = cos(RAD(cam_pitch));
    float sp = sin(RAD(cam_pitch));
    float y1 = p.y * cp - p.z * sp;
    float z1 = p.y * sp + p.z * cp;
    float x1 = p.x;

    // 再做相机围绕 Z 轴的偏航（绕 Z 轴）
    float cy = cos(RAD(cam_yaw));
    float sy = sin(RAD(cam_yaw));
    float x2 = x1 * cy - y1 * sy;
    float y2 = x1 * sy + y1 * cy;
    float z2 = z1 + cam_distance; // 将相机沿 z 方向后移一个距离以产生透视

    Point3D out = {x2, y2, z2};
    return out;
}

/**
 * @brief 3D点透视投影到2D屏幕
 * @param p 旋转后的3D点
 * @return 投影后的2D点
 */
Point2D project_point(Point3D p) {
    Point2D res;
    // 透视缩放（z越大，投影越小）
    float scale = PROJECTION_SCALE / (PROJECTION_SCALE + p.z);
    // 转换到屏幕坐标（Y轴向下为正）
    res.x = CENTER_X + (int)(p.x * scale);
    res.y = CENTER_Y + (int)(p.y * scale);
    return res;
}

/**
 * @brief 绘制坐标轴端点标记（区分不同轴）
 * @param p 2D端点坐标
 * @param axis 轴标识（0:X, 1:Y, 2:Z）
 */
void draw_axis_marker(Point2D p, uint8_t axis) {
    switch(axis) {
        case 0:  // X轴：绘制右向箭头
            OLED_DrawLine(p.x, p.y, p.x+3, p.y-2);
            OLED_DrawLine(p.x, p.y, p.x+3, p.y+2);
            break;
        case 1:  // Y轴：绘制上向箭头
            OLED_DrawLine(p.x, p.y, p.x-2, p.y-3);
            OLED_DrawLine(p.x, p.y, p.x+2, p.y-3);
            break;
        case 2:  // Z轴：绘制圆点
            OLED_DrawLine(p.x-1, p.y-1, p.x+1, p.y+1);
            OLED_DrawLine(p.x-1, p.y+1, p.x+1, p.y-1);
            break;
    }
}

/**
 * @brief 绘制动态三维坐标系
 */
void draw_3d_coordinate() {
    // 1. 旋转三维坐标点（传感器坐标 -> 世界坐标）
    Point3D o_rot = rotate_point(origin, Roll, Pitch, Yaw);
    Point3D x_rot = rotate_point(x_axis, Roll, Pitch, Yaw);
    Point3D y_rot = rotate_point(y_axis, Roll, Pitch, Yaw);
    Point3D z_rot = rotate_point(z_axis, Roll, Pitch, Yaw);

    // 可选：根据相机模式动态调整视角
    if (cam_mode == 1) {
        // 自动环绕：缓慢改变相机偏航角以获得动态视角
        cam_yaw += 1.0f; // 每帧旋转1度（速度可调）
        if (cam_yaw >= 360.0f) cam_yaw -= 360.0f;
    } else if (cam_mode == 2) {
        // 跟随传感器：将相机角度设置为传感器的偏航与俯仰（简化）
        cam_yaw = -Yaw;   // 取负可以获得更直观的跟随效果
        cam_pitch = Pitch;
    }

    // 2. 将世界坐标变换到相机视角（摄像机坐标系）
    Point3D o_cam = apply_camera_transform(o_rot);
    Point3D x_cam = apply_camera_transform(x_rot);
    Point3D y_cam = apply_camera_transform(y_rot);
    Point3D z_cam = apply_camera_transform(z_rot);

    // 3. 投影到二维平面
    Point2D o_proj = project_point(o_cam);
    Point2D x_proj = project_point(x_cam);
    Point2D y_proj = project_point(y_cam);
    Point2D z_proj = project_point(z_cam);
    
    // 3. 清屏并绘制
    OLED_Clear();
    
    // 绘制坐标轴（移除多余的颜色参数）
    OLED_DrawLine(o_proj.x, o_proj.y, x_proj.x, x_proj.y);  // X轴
    OLED_DrawLine(o_proj.x, o_proj.y, y_proj.x, y_proj.y);  // Y轴
    OLED_DrawLine(o_proj.x, o_proj.y, z_proj.x, z_proj.y);  // Z轴
    
    // 绘制端点标记
    draw_axis_marker(x_proj, 0);
    draw_axis_marker(y_proj, 1);
    draw_axis_marker(z_proj, 2);

    // 绘制简单地面网格（稀疏）以增强深度感知
    for (int gx = -2; gx <= 2; gx++) {
        Point3D p1 = {gx * 10.0f, -20.0f, 0.0f};
        Point3D p2 = {gx * 10.0f, 20.0f, 0.0f};
        Point2D pp1 = project_point(apply_camera_transform(p1));
        Point2D pp2 = project_point(apply_camera_transform(p2));
        OLED_DrawLine(pp1.x, pp1.y, pp2.x, pp2.y);
    }
    for (int gy = -2; gy <= 2; gy++) {
        Point3D p1 = {-20.0f, gy * 10.0f, 0.0f};
        Point3D p2 = {20.0f, gy * 10.0f, 0.0f};
        Point2D pp1 = project_point(apply_camera_transform(p1));
        Point2D pp2 = project_point(apply_camera_transform(p2));
        OLED_DrawLine(pp1.x, pp1.y, pp2.x, pp2.y);
    }
    
    // 显示姿态数据
    OLED_Printf(0, 0, OLED_8X16, "Roll :%+06.1f", Roll);
    OLED_Printf(0, 16, OLED_8X16, "Yaw  :%+06.1f", Yaw);
    OLED_Printf(0, 32, OLED_8X16, "Pitch:%+06.1f", Pitch);
    
    OLED_Update();
}

int main(void) {
    OLED_Init();
    Serial_Init();
    MPU6050_DMPInit();
    Timer_Init();
    
    OLED_ShowString(0, 48, "3D Coordinate", OLED_8X16);  // 底部提示文本
    Delay_ms(1000);  // 初始化显示延迟
    
    while(1) {
        draw_3d_coordinate();  // 主循环绘制三维坐标系
    }
}

/**
 * @brief TIM2定时中断函数，更新姿态数据
 */
void TIM2_IRQHandler(void) {
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
        MPU6050_ReadDMP(&Pitch, &Roll, &Yaw);  // 读取姿态角
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
