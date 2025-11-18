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
					
						OLED_Printf(0, 0, OLED_8X16, "LED0:");
						OLED_Printf(0, 16, OLED_8X16, "LED1:");
						OLED_Printf(0, 32, OLED_8X16, "LED2:");
						OLED_Printf(0, 48, OLED_8X16, "LED3:");
								
						OLED_Update();
					
						MPU6050_ENABLE = 0;
					
						LED_SetMode(LED_ALL_OFF_Mode);
						LED_SetMode(LED_SET_BRIGHT_MODE);
			
						break;
					
					case MISSION_B:

						OLED_Clear();
								
						OLED_Printf(0, 0, OLED_8X16, "N0:");
						OLED_Printf(0, 16, OLED_8X16, "N1:");
						OLED_Printf(0, 32, OLED_8X16, "N2:");
						OLED_Printf(0, 48, OLED_8X16, "N3:");
					
						OLED_Printf(56, 0, OLED_8X16, "F0:");
						OLED_Printf(56, 16, OLED_8X16, "F1:");
						OLED_Printf(56, 32, OLED_8X16, "F2:");
						OLED_Printf(56, 48, OLED_8X16, "F3:");
					
						OLED_Update();
					
						MPU6050_ENABLE = 0;
					
						LED_SetMode(LED_ALL_OFF_Mode);
					
						LED_SetMode(LED_SET_FLASH_MODE);
					
						break;
					
					case MISSION_C:

						OLED_Clear();
								
						OLED_Update();
					
						MPU6050_ENABLE = 1;
					
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
								
									OLED_Printf(40, 0, OLED_8X16, "%3d", B0 );
								
									OLED_Update();
								
									break;
								
								case 1:
									B1 = TempBright;
								
									OLED_Printf(40, 16, OLED_8X16, "%3d", B1 );
								
									OLED_Update();
								
									break;
								
								case 2:
									B2 = TempBright;
								
									OLED_Printf(40, 32, OLED_8X16, "%3d", B2 );
								
									OLED_Update();
									break;
								
								case 3:
									B3 = TempBright;
								
									OLED_Printf(40, 48, OLED_8X16, "%3d", B3 );
								
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
			
			
			
				break;
			
			case MISSION_C:
			
				break;
			
			case MISSION_D:
			
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
				
		//		BlueSerial_Printf("[plot,%f,%f,%f]", Roll, Yaw, Pitch);
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
