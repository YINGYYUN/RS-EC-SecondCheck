#include "stm32f10x.h"                  // Device header

#include "Delay.h"
#include "Timer.h"
#include "KalmanFilter.h"

#include "LED.h"
#include "MPU6050.h"
#include "Serial.h"
#include "OLED.h"
#include "Servo.h"

#include <math.h>

#define M_PI 3.14159265358979323846f

// 定义卡尔曼滤波器实例（俯仰角）
KalmanFilterTypeDef kf_pitch;
// 采样时间（TIM1 更新中断为1ms）
const float dt = 0.001f;

float Pitch, Roll, Yaw;
uint16_t Angle = 90 ;

int main(void)
{
	LED_Init();
    OLED_Init();
    Serial_Init();
	MPU6050_Init();
//	MPU6050_DMPInit();
	Servo_Init();
	
	Timer_Init();
	
	KalmanFilter_Init(&kf_pitch, 0.001f, 0.003f, 0.03f);
	
	LED_SetMode(LED_SlowFlashMode);
	
	OLED_ShowString(0, 0, "MPU6050", OLED_8X16);
    OLED_ShowString(0, 16, "Pitch:", OLED_8X16);  // 显示Pitch角标签
	
    OLED_Update();
	
	Servo_SetAngle(Angle);
	
    while(1)
    {
//		OLED_Printf(0, 16, OLED_8X16, "Pitch:%+06.1f", Pitch);
//		OLED_Printf(0, 32, OLED_8X16, "Roll :%+06.1f", Roll);
//		OLED_Printf(0, 48, OLED_8X16, "Yaw  :%+06.1f", Yaw);
//		
//		OLED_Update();
//		
//		Angle = (Yaw + 180.0f) * 180.0f / 360.0f;
//		
//		if(Angle < 0) Angle = 0;
//		else if(Angle > 180) Angle = 180;
//		
//		if(Angle <= 10.0 || 170.0<= Angle)	{LED_SetMode(LED_SlowFlashMode);}
//		else 							{LED_SetMode(LED_FastFlashMode);}
//		Servo_SetAngle(Angle);
		
		// 在主循环中更新OLED显示（使用滤波后的角度）
        OLED_Printf(48, 16, OLED_8X16, "%+06.1f", Pitch);
		
        OLED_Update();
        
	// 根据Pitch控制舵机（这里暂时使用Pitch角演示，实际可替换为Yaw角逻辑）
	Angle = (uint16_t)(Pitch + 90.0f);  // 映射角度到0~180范围
        if(Angle < 0) Angle = 0;
        else if(Angle > 180) Angle = 180;
        
        // 增加角度波动滤波，避免频繁切换模式
	static float lastAngle = 0;
	if(fabs(Angle - lastAngle) > 2.0f)  // 角度变化超过2度才更新
	{
		lastAngle = Angle;
		if(Angle <= 10 || Angle >= 170)
			LED_SetMode(LED_SlowFlashMode);
		else
			LED_SetMode(LED_FastFlashMode);
		// 当角度有显著变化时更新舵机角度
		Servo_SetAngle(Angle);
	}
		
    }
}

void TIM1_UP_IRQHandler(void)
{
	//检查标志位
	if (TIM_GetITStatus(TIM1,TIM_IT_Update) == SET )
	{
		int16_t AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
        // 读取MPU6050原始数据（现有函数）
        MPU6050_GetData(&AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ);
        
        // 1. 计算加速度计角度（弧度）
        float accel_angle = KalmanFilter_GetAccelAngle(AccX, AccY, AccZ);
        
        // 2. 转换陀螺仪数据为弧度/秒
        // （MPU6050配置为±2000°/s，灵敏度16.4 LSB/(°/s)，转换公式：弧度/秒 = (原始值 / 16.4) * π/180）
        float gyro_rate = (float)GyroX / 16.4f * M_PI / 180.0f;
        
	// 3. 卡尔曼滤波更新，得到滤波后的角度（弧度）
	float filtered_angle = KalmanFilter_Update(&kf_pitch, accel_angle, gyro_rate, dt);
	// 把滤波后的角度转换为角度制并写回全局变量，供主循环显示与舵机控制使用
	Pitch = filtered_angle * 180.0f / M_PI; // Pitch 单位现在为度
	// 实时更新舵机角度（在中断中更新CCR通常是安全且延迟最低的方式）
	Angle = (uint16_t)(Pitch + 90.0f);
	if (Angle < 0) Angle = 0;
	else if (Angle > 180) Angle = 180;
	Servo_SetAngle((float)Angle);
	LED_Tick();
//		MPU6050_ReadDMP(&Pitch, &Roll, &Yaw);
		//清除标志位
		TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
	}
}
