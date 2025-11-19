#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "Serial.h"
#include "Timer.h"

#include <math.h>

float Pitch, Roll, Yaw;

// 绘制3D坐标系到OLED的辅助函数声明
static void OLED_Draw3DAxes(float roll_deg, float pitch_deg, float yaw_deg);

int main(void)
{
    OLED_Init();
    Serial_Init();
	MPU6050_DMPInit();
	Timer_Init();
	
	OLED_ShowString(0, 0, "MPU6050", OLED_8X16);
	
    while(1)
    {
		OLED_Printf(0, 0, OLED_8X16, "R:%+02.3f", Roll);
		OLED_Printf(0, 16, OLED_8X16, "Y:%+02.3f", Yaw);
		OLED_Printf(0, 32, OLED_8X16, "P:%+02.3f", Pitch);

		/* 在右侧区域绘制3D坐标系（根据传感器姿态） */
		OLED_ClearArea(80, 0, 48, 64); // 清除右侧区域
		OLED_Draw3DAxes(Roll, Yaw, Pitch);
		
		OLED_Update();
		
//		BlueSerial_Printf("[plot,%f,%f,%f]", Roll, Yaw, Pitch);
		Serial_Printf("%f,%f,%f\r\n", Roll, Yaw, Pitch);
    }
}


/**
 * 将三维坐标轴按当前姿态旋转并投影到OLED屏幕（单色）
 * roll_deg, pitch_deg, yaw_deg 单位为度
 */
static void OLED_Draw3DAxes(float roll_deg, float pitch_deg, float yaw_deg)
{
	/* 参数 */
	const float L = 20.0f;      // 轴长度（3D坐标单位）
	const float f = 40.0f;      // 透视焦距（像素比例）
	const float z_offset = 60.0f; // 将物体向后移动，避免除以负值

	/* 屏幕中心（投影原点），放在右侧区域中心 */
	const int cx = 104; // 80 + 24
	const int cy = 32;

	/* 角度转换为弧度 */
	float roll = roll_deg * 3.1415926f / 180.0f;
	float pitch = pitch_deg * 3.1415926f / 180.0f;
	float yaw = yaw_deg * 3.1415926f / 180.0f;

	/* 计算旋转矩阵 R = Rz(yaw) * Ry(pitch) * Rx(roll) */
	float cr = cosf(roll), sr = sinf(roll);
	float cp = cosf(pitch), sp = sinf(pitch);
	float cyaw = cosf(yaw), syaw = sinf(yaw);

	float R00 = cyaw * cp;
	float R01 = cyaw * sp * sr - syaw * cr;
	float R02 = cyaw * sp * cr + syaw * sr;

	float R10 = syaw * cp;
	float R11 = syaw * sp * sr + cyaw * cr;
	float R12 = syaw * sp * cr - cyaw * sr;

	float R20 = -sp;
	float R21 = cp * sr;
	float R22 = cp * cr;

	/* 三个轴的基向量（X红，Y绿，Z蓝 — 单色显示为线条） */
	float ax[3][3] = {{L,0,0},{0,L,0},{0,0,L}};

	/* 投影并绘线 */
	for (int i = 0; i < 3; i++)
	{
		/* 旋转 */
		float x3 = R00 * ax[i][0] + R01 * ax[i][1] + R02 * ax[i][2];
		float y3 = R10 * ax[i][0] + R11 * ax[i][1] + R12 * ax[i][2];
		float z3 = R20 * ax[i][0] + R21 * ax[i][1] + R22 * ax[i][2];

		/* 简单透视投影 */
		float zc = z3 + z_offset;
		if (zc < 5.0f) zc = 5.0f; // 避免太接近导致放大

		int sx = (int)(cx + (f * x3) / zc);
		int sy = (int)(cy - (f * y3) / zc); // 屏幕y向下，故这里取负

		/* 原点屏幕坐标 */
		int ox = cx;
		int oy = cy;

		/* 画轴线 */
		OLED_DrawLine(ox, oy, sx, sy);

		/* 画箭头（两个短线）*/
		int dx = sx - ox;
		int dy = sy - oy;
		float len = sqrtf((float)(dx*dx + dy*dy));
		if (len > 0.001f)
		{
			float ux = dx / len;
			float uy = dy / len;
			/* 箭头大小 */
			float ah = 4.0f;
			/* 两个方向，旋转±30度 */
			float ang = 30.0f * 3.1415926f / 180.0f;
			float cang = cosf(ang), sang = sinf(ang);

			int ax1 = (int)(sx - ah * (ux * cang - uy * sang));
			int ay1 = (int)(sy - ah * (ux * sang + uy * cang));
			int ax2 = (int)(sx - ah * (ux * cang + uy * sang));
			int ay2 = (int)(sy + ah * (ux * sang - uy * cang));

			OLED_DrawLine(sx, sy, ax1, ay1);
			OLED_DrawLine(sx, sy, ax2, ay2);
		}
	}
}


/**
  * @brief  TIM2定时中断函数，复制到main
  * @param  无
  * @retval 无
  */
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)    //判断是否为定时器TIM2更新中断
	{
		MPU6050_ReadDMP(&Pitch, &Roll, &Yaw);
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);    //清除定时器TIM2更新中断标志位 
	}
}
