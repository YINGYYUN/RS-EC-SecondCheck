#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Timer.h"
#include "OLED.h"
#include "LED.h"
#include "Serial.h"
#include "MPU6050.h"
#include "Servo.h"
#include "Key.h"

#include <math.h>
#include <string.h>


/* ========= [START] MPU6050解算初始化模块 [START] =========*/
int16_t AX, AY, AZ, GX, GY, GZ;

//定时中断重叠标志位
//uint8_t TimerErrorFlag;

//定时中断执行时长
//uint16_t TimerCount;

uint8_t MPU6050_ENABLE = 0;

float RollAcc;    		// 加速度计计算的横滚角
float RollGyro;   		// 陀螺仪积分的横滚角
float Roll;       		// 融合后的横滚角

float Yaw = 0;			//偏航角

float PitchAcc;			//加速度计算的俯仰角
float PitchGyro;		//陀螺仪积分的俯仰角
float Pitch;			//融合后的俯仰角

// 绘制3D坐标系到OLED的辅助函数声明
static void OLED_Draw3DAxes(float roll_deg, float pitch_deg, float yaw_deg);

/*========= [END] MPU6050解算初始化模块 [END] =========*/


//舵机角度
float Angle = 90.0f;
uint8_t KeyNum,nm;


int main(void)
{

	LED_Init();
	Serial_Init();
	MPU6050_Init();
	Servo_Init();
	Key_Init();
	
	Timer_Init();
	
	LED_SetMode(LED_ALL_OFF_Mode);
	
	Serial_RxFlag = 0;
	
	uint8_t KeyNum;
	
	/* =========[SRART] 菜单初始化模块 [START] =========*/
	OLED_Init();
	
	char Menu_Main[][12] = {"MISSION_A" ,"MISSION_B" ,"MISSION_C" ,"MISSION_D" ,"MISSION_E"}; 
	
	//宏定义：页面
	#define MENU					0                 
	#define MISSION_A				1
	#define MISSION_B				2
	#define MISSION_C				3
	#define MISSION_D				4
	#define MISSION_E				5
          
	//OLED显示状态机(包括工作进行确认)
	uint8_t	CURRENT_MODE = MENU;
	//主菜单光标位置 
	//0~4 对应 A~E
	uint8_t Location_MENU = 0;
	
	//OLED_8X16		宽8像素，高16像素
	//
	//OLED_6X8		宽6像素，高8像素
	//	  坐标轴定义：
	//  左上角为(0, 0)点
	//  横向向右为X轴，取值范围：0~127
	//  纵向向下为Y轴，取值范围：0~63
	//  
	//        0             X轴           127 
	//       .------------------------------->
	//     0 |
	//       |
	//       |
	//       |
	//   Y轴 |
	//       |
	//       |
	//       |
	//    63 |
	//       v
	
	//OLED显示初始化
	OLED_Clear();
	
	OLED_Printf(6, 0, OLED_6X8, Menu_Main[0]);
	OLED_Printf(6, 10, OLED_6X8, Menu_Main[1]);
	OLED_Printf(6, 20, OLED_6X8, Menu_Main[2]);
	OLED_Printf(6, 30, OLED_6X8, Menu_Main[3]);
	OLED_Printf(6, 40, OLED_6X8, Menu_Main[4]);

	OLED_Printf(0, Location_MENU * 10, OLED_6X8, ">");
	
	OLED_Update();	
	
	Serial_SendString("[INFO]MENU_READY\r\n");
	/* =========[END] 菜单初始化模块 [END] =========*/

	
	
	Serial_SendString("[INFO]LED_ALL_OFF_Mode\r\n");
	
	while(1)
	{	
		
		KeyNum  = Key_GetNum();		
		/* =========[SRART] 按键处理及菜单响应模块 [START] =========*/
		
		//上键
		if (CURRENT_MODE == MENU && KeyNum == 1)
		{
			OLED_Printf(0, Location_MENU * 10, OLED_6X8, " ");
			Location_MENU = (Location_MENU - 1 + 5) % 5;
			OLED_Printf(0, Location_MENU * 10, OLED_6X8, ">");
			
			OLED_Update();
		}
		
		//下键
		else if (CURRENT_MODE == MENU && KeyNum == 2)
		{
			OLED_Printf(0, Location_MENU * 10, OLED_6X8, " ");
			Location_MENU = (Location_MENU + 1 ) % 5;
			OLED_Printf(0, Location_MENU * 10, OLED_6X8, ">");
			
			OLED_Update();
		}
		
		//确认+退出键
		else if (KeyNum == 3)
		{
			//处在主菜单[一级]
			if(CURRENT_MODE == MENU)
			{
				CURRENT_MODE = Location_MENU + 1;
				
				switch(CURRENT_MODE)
				{
					case MISSION_A:

						OLED_Clear();
					
						B0 = 0;
						B1 = 0;
						B2 = 0;
						B3 = 0;
					
					
						OLED_Printf(0, 0, OLED_8X16, "LED0:%03d", B0);
						OLED_Printf(0, 16, OLED_8X16, "LED1:%03d", B1);
						OLED_Printf(0, 32, OLED_8X16, "LED2:%03d", B2);
						OLED_Printf(0, 48, OLED_8X16, "LED3:%03d", B3);
								
						OLED_Update();
					
						MPU6050_ENABLE = 0;
					
						LED_SetMode(LED_ALL_OFF_Mode);
						LED_SetMode(LED_SET_BRIGHT_MODE);
			
						break;
					
					case MISSION_B:
						
						B0 = 0;
						B1 = 0;
						B2 = 0;
						B3 = 0;

						OLED_Clear();
								
						OLED_Printf(0, 0, OLED_8X16, "N0:%04dF0:%04d", B0, 1000 - B0);
						OLED_Printf(0, 16, OLED_8X16, "N1:%04dF1:%04d", B1, 1000 - B1);
						OLED_Printf(0, 32, OLED_8X16, "N2:%04dF2:%04d", B2, 1000 - B2);
						OLED_Printf(0, 48, OLED_8X16, "N3:%04dF3:%04d", B3, 1000 - B3);
					
						OLED_Update();
					
						MPU6050_ENABLE = 0;
					
						LED_SetMode(LED_ALL_OFF_Mode);
					
						LED_SetMode(LED_SET_FLASH_MODE);
					
						break;
					
					case MISSION_C:

						OLED_Clear();
								
						OLED_Update();
					
						MPU6050_ENABLE = 0;
					
						LED_SetMode(LED_ALL_OFF_Mode);
					
						break;
					
					case MISSION_D:

						OLED_Clear();
								
						OLED_Update();
					
						MPU6050_ENABLE = 1;
					
						LED_SetMode(LED_ALL_OFF_Mode);
					
						break;
					
					case MISSION_E:
						
						OLED_Clear();
								
						OLED_Update();
					
						MPU6050_ENABLE = 1;
					
						LED_SetMode(LED_ALL_OFF_Mode);						
						LED_SetMode(LED_PIN_0_SlowFlash_Mode);
					
						break;
					
					default:
						//留一手
						break;
				}
				

			}
			//处在子菜单[二级]
			else 
			{
				LED_SetMode(LED_ALL_OFF_Mode);
				CURRENT_MODE = MENU;
		
				OLED_Clear();

				OLED_Printf(6, 0, OLED_6X8, Menu_Main[0]);
				OLED_Printf(6, 10, OLED_6X8, Menu_Main[1]);
				OLED_Printf(6, 20, OLED_6X8, Menu_Main[2]);
				OLED_Printf(6, 30, OLED_6X8, Menu_Main[3]);
				OLED_Printf(6, 40, OLED_6X8, Menu_Main[4]);

				OLED_Printf(0, Location_MENU * 10, OLED_6X8, ">");
				
				OLED_Update();	
			}
		}		
		/* =========[END] 按键处理及菜单响应模块 [END] =========*/
		
		
		/* =========[SRART] 功能响应模块 [START] =========*/
		switch(CURRENT_MODE)
		{
			case MENU:
				//???
				break;
			
			case MISSION_A:
				if (Serial_RxFlag == 1)
				{
				//输入示例@LED0%80 (\r\n)
				
				Serial_Printf("[INFO]Received: %s\r\n", Serial_RxPacket);//接收文本直接回传上位机

					if (strstr(Serial_RxPacket, "LED") != NULL) 
					{
						int16_t LED_Num = -1;
						int16_t TempBright = 0;
						if (sscanf(Serial_RxPacket, "LED%hd%%%hd", &LED_Num, &TempBright) == 2)
						{
							if (TempBright >= 100)TempBright = 100;
							if (TempBright <= 0)TempBright = 0;
							uint8_t Flag_Found = 1;

							switch(LED_Num)
							{
								case 0:
									B0 = TempBright;
								
									OLED_Printf(40, 0, OLED_8X16, "%03d", B0 );
								
									OLED_Update();
								
									break;
								
								case 1:
									B1 = TempBright;
								
									OLED_Printf(40, 16, OLED_8X16, "%03d", B1 );
								
									OLED_Update();
								
									break;
								
								case 2:
									B2 = TempBright;
								
									OLED_Printf(40, 32, OLED_8X16, "%03d", B2 );
								
									OLED_Update();
									break;
								
								case 3:
									B3 = TempBright;
								
									OLED_Printf(40, 48, OLED_8X16, "%03d", B3 );
								
									OLED_Update();
								
									break;
								
								default:
									Flag_Found = 0;
									Serial_SendString("[INFO]INVALID_NUM\r\n");//状态回传上位机
									break;
							}
							if(Flag_Found)Serial_Printf("[INFO]Set_LED%d:%d\r\n", LED_Num, (int)B3);//状态回传上位机
						}
						else 
						{
							Serial_SendString("[INFO]ERROR_COMMAND\r\n");//状态回传上位机
						}
						
					}
					else {
						Serial_SendString("[INFO]ERROR_COMMAND\r\n");//状态回传上位机
					}
				Serial_RxFlag = 0;
				}
			
			
				break;
			
			case MISSION_B:
			
				if (Serial_RxFlag == 1)
				{
				Serial_Printf("[INFO]Received: %s\r\n", Serial_RxPacket);//接收文本直接回传上位机

					//更简单的判定
					if (strstr(Serial_RxPacket, "LED") != NULL) {
						int16_t LED_Num = -1;
						int16_t TempBright = 0;
						if (sscanf(Serial_RxPacket, "LED%hd%%%hd", &LED_Num, &TempBright) == 2)
						{
							if (TempBright >= 1000)TempBright = 100;
							if (TempBright <= 0)TempBright = 0;
							uint8_t Flag_Found = 1;
							switch(LED_Num)
							{
								case 0:
									B0 = TempBright;
								
									OLED_Printf(0, 0, OLED_8X16, "N0:%04dF0:%04d", B0, 1000 - B0);								
									OLED_Update();
								
									break;
								
								case 1:
									B1 = TempBright;
								
									OLED_Printf(0, 16, OLED_8X16, "N0:%04dF0:%04d", B1, 1000 - B1);								
									OLED_Update();
								
									break;
								
								case 2:
									B2 = TempBright;
								
									OLED_Printf(0, 32, OLED_8X16, "N0:%04dF0:%04d", B2, 1000 - B2);								
									OLED_Update();
								
									break;
								
								case 3:
									B3 = TempBright;
								
									OLED_Printf(0, 48, OLED_8X16, "N0:%04dF0:%04d", B3, 1000 - B3);
									OLED_Update();
								
									break;
								
								default:
									Flag_Found = 0;
									Serial_SendString("[INFO]ERROR_COMMAND\r\n");//状态回传上位机
									break;
							}
							if(Flag_Found)Serial_Printf("[INFO]Set_LED%d:%d\r\n",LED_Num , (int)B3);//状态回传上位机
						}
						else 
						{
							Serial_SendString("[INFO]ERROR_COMMAND\r\n");//状态回传上位机
						}
					}
					else {
						Serial_SendString("[INFO]ERROR_COMMAND\r\n");//状态回传上位机
					}
				Serial_RxFlag = 0;
				}
			
			
				break;
			
			case MISSION_C:
				MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);

				float AX_g, AY_g, AZ_g;
				float GX_dps, GY_dps, GZ_dps;

				AX_g = AX / 32768.0 * 16 * 9.8;
				AY_g = AY / 32768.0 * 16 * 9.8;
				AZ_g = AZ / 32768.0 * 16 * 9.8;
				
				GX_dps = GX / 32768.0 * 2000;
				GY_dps = GY / 32768.0 * 2000;
				GZ_dps = GZ / 32768.0 * 2000;
								
				OLED_Printf(0, 0, OLED_8X16, "%+05.2f", AX_g);					//OLED显示数据
				OLED_Printf(0, 16, OLED_8X16, "%+05.2f", AY_g);
				OLED_Printf(0, 32, OLED_8X16, "%+05.2f", AZ_g);
				OLED_Printf(57, 0, OLED_8X16, "%+05.2f", GX_dps);
				OLED_Printf(57, 16, OLED_8X16, "%+05.2f", GY_dps);
				OLED_Printf(57, 32, OLED_8X16, "%+05.2f", GZ_dps);

				OLED_Update();
			
			
				break;
			
			case MISSION_D:
				OLED_Printf(0, 0, OLED_8X16, "R:%+02.3f", Roll);
				OLED_Printf(0, 16, OLED_8X16, "Y:%+02.3f", Yaw);
				OLED_Printf(0, 32, OLED_8X16, "P:%+02.3f", Pitch);
			
				/* 在右侧区域绘制3D坐标系（根据传感器姿态） */
				OLED_ClearArea(80, 0, 48, 64); // 清除右侧区域
				OLED_Draw3DAxes(Roll, Yaw, Pitch);
				
				OLED_Update();
			
				//BlueSerial_Printf("[plot,%f,%f,%f]", Roll, Yaw, Pitch);
				Serial_Printf("%f,%f,%f\r\n", Roll, Yaw, Pitch);
			
				break;
			
			case MISSION_E:
				
				OLED_Printf(0, 0, OLED_8X16, "Roll :%+02.3f", Roll);
				OLED_Printf(0, 16, OLED_8X16, "Yaw  :%+02.3f", Yaw);
				OLED_Printf(0, 32, OLED_8X16, "Pitch:%+02.3f", Pitch);
				
				OLED_Update();
				Angle = (Yaw + 180.0f) * 180.0f / 360.0f;
				
				if (Angle < 0.0f) Angle = 0.0f;
				if (Angle > 180.0f) Angle = 180.0f;
				
				if ((Angle <= 15.0f || 165.0f <= Angle) && LED_Mode != LED_PIN_0_FastFlash_Mode )
				{
					LED_SetMode(LED_PIN_0_FastFlash_Mode);
					Serial_SendString("[INFO]LED_FastFlashMode\r\n");
				}
				else if((10.0f <= Angle && Angle<= 170.0f) && LED_Mode != LED_PIN_0_SlowFlash_Mode )
				{
					LED_SetMode(LED_PIN_0_SlowFlash_Mode);
					Serial_SendString("[INFO]LED_SlowFlashMode\r\n");			
				}
				
				//BlueSerial_Printf("[plot,%f,%f,%f]", Roll, Yaw, Pitch);
				Serial_Printf("%f,%f,%f,%f\r\n", Roll, Yaw, Pitch, Angle);
				
				Servo_SetAngle(Angle);
			
				break;
			
			default:
				//留一手
				break;	
		}
		/* =========[END] 功能响应模块 [END] =========*/
		
		

	}
	
}


/* =========[START] 坐标系计算模块 [START] =========*/


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


/* =========[END] 坐标系计算模块 [END] =========*/


//1ms的定时中断
void TIM1_UP_IRQHandler(void)
{
	//检查标志位
	if (TIM_GetITStatus(TIM1,TIM_IT_Update) == SET )
	{
		//清除标志位
		TIM_ClearITPendingBit(TIM1,TIM_IT_Update);

		LED_Tick();

		Key_Tick();

		if (MPU6050_ENABLE)
		{
			MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
			
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
			
	//		if (TIM_GetITStatus(TIM1,TIM_IT_Update) == SET )
	//		{
	//			//中断重叠标志位
	//			TimerErrorFlag = 1;
				//清除标志位
				TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
	//		}
	//		TimerCount = TIM_GetCounter(TIM1);
		}
	}
}
