#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Timer.h"
#include "OLED.h"
//#include "LED.h"
#include "Serial.h"
#include "MPU6050.h"
//#include "Servo.h"
#include <math.h>

int16_t AX, AY, AZ, GX, GY, GZ;

//定时中断重叠标志位
uint8_t TimerErrorFlag;

//定时中断执行时长
uint16_t TimerCount;

float RollAcc;    		// 加速度计计算的横滚角
float RollGyro;   		// 陀螺仪积分的横滚角
float Roll;       		// 融合后的横滚角

float Yaw = 0;			//偏航角

float PitchAcc;			//加速度计算的俯仰角
float PitchGyro;		//陀螺仪积分的俯仰角
float Pitch;			//融合后的俯仰角

// 绘制3D坐标系到OLED的辅助函数声明
static void OLED_Draw3DAxes(float roll_deg, float pitch_deg, float yaw_deg);

int main(void)
{
	OLED_Init();
//	LED_Init();
	Serial_Init();
	MPU6050_Init();
//	Servo_Init();
	
	Timer_Init();
	
//	LED_SetMode(LED_OFFMode);
	

	
	while(1)
	{
		
		
		OLED_Printf(0, 0, OLED_8X16, "%+06d", AX);
		OLED_Printf(0, 16, OLED_8X16, "%+06d", AY);
		OLED_Printf(0, 32, OLED_8X16, "%+06d", AZ);
		OLED_Printf(64, 0, OLED_8X16, "%+06d", GX);
		OLED_Printf(64, 16, OLED_8X16, "%+06d", GY);
		OLED_Printf(64, 32, OLED_8X16, "%+06d", GZ);
		
		OLED_Printf(0, 48, OLED_8X16, "Flag:%1d", TimerErrorFlag);
		OLED_Printf(64, 48, OLED_8X16, "C:%05d", TimerCount);

		/* 在右侧区域绘制3D坐标系（根据传感器姿态） */
		OLED_ClearArea(80, 0, 48, 64); // 清除右侧区域
		OLED_Draw3DAxes(Roll, Yaw, Pitch);
		
		OLED_Update();
		
//		BlueSerial_Printf("[plot,%f,%f,%f]", Roll, Yaw, Pitch);
		Serial_Printf("%f,%f,%f\r\n", Roll, Yaw, Pitch);
	}
	
}

//1ms的定时中断
void TIM1_UP_IRQHandler(void)
{
	//检查标志位
	if (TIM_GetITStatus(TIM1,TIM_IT_Update) == SET )
	{
		//清除标志位
		TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
		//保证数据的及时读取
		MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
//		LED_Tick();
		
		//校准零飘
		GX += 55;
		GY += 18;
		GZ += 10;		
		
		// 横滚角计算
        RollAcc = atan2(AY, AZ) / 3.14159 * 180;  				// 横滚角（绕X轴）
        RollGyro = Roll + GX / 32768.0 * 2000 * 0.001;  		// 陀螺仪X轴积分
        Roll = 0.001 * RollAcc + (1 - 0.001) * RollGyro;  		// 相同互补滤波算法
		
		// 偏航角：仅陀螺仪积分（无加速度计校准，会漂移）
        Yaw += GZ / 32768.0 * 2000 * 0.001;  // 仅积分，无校准
		
		// 俯仰角计算
		PitchAcc = -atan2(AX, AZ) / 3.14159 * 180;  			// 俯仰角（绕Y轴）
        PitchGyro = Pitch + GY / 32768.0 * 2000 * 0.001;  		// 陀螺仪积分（2000是量程，0.001是1ms采样间隔）
        Pitch = 0.001 * PitchAcc + (1 - 0.001) * PitchGyro;  	// 互补滤波
		
//		//俯仰角
//		AngleAcc = -atan2(AX, AZ) / 3.14159 * 180;
//		
//		//角速度积分
//		AngleGyro = Angle + GY / 32768.0 * 2000 * 0.001;
//		
//		float Alpha  = 0.001;
//		Angle = Alpha * AngleAcc + (1 - Alpha) * AngleGyro;
		
		if (TIM_GetITStatus(TIM1,TIM_IT_Update) == SET )
		{
			//中断重叠标志位
			TimerErrorFlag = 1;
			//清除标志位
			TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
		}
		TimerCount = TIM_GetCounter(TIM1);
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
