#include "stm32f10x.h"  // Device header
#include "Delay.h"
#include "Timer.h"
#include "LED.h"
#include "OLED.h"
#include "Serial.h"

#include <string.h>



int main(void)
{		
	LED_Init();
	OLED_Init();
	Serial_Init();
	
	Timer_Init();

	OLED_ShowString(1,1,"LED0:");	
	OLED_ShowString(2,1,"LED1:");
	OLED_ShowString(3,1,"LED2:");
	OLED_ShowString(4,1,"LED3:");
		
	OLED_ShowNum(1,6,B0,3);
	OLED_ShowNum(2,6,B1,3);
	OLED_ShowNum(3,6,B2,3);
	OLED_ShowNum(4,6,B3,3);
	
	Serial_SendString("SYSTEM_READY\r\n");
	
	Serial_RxFlag = 0;
	//查询接收，演示用，不封装
	while(1)
	{
		if (Serial_RxFlag == 1)
		{
			//输入示例@LED0%80 (\r\n)
			
			Serial_Printf("[INFO]Received: %s\r\n", Serial_RxPacket);//接收文本直接回传上位机
			/*初版的复杂判定*/
//					if (strstr(Serial_RxPacket, "LED0%") != NULL) {
//						int16_t speed;
//						sscanf(Serial_RxPacket, "LED0%%%hd", &TempBright);
//						if (TempBright >= 100)TempBright = 100;
//						if (TempBright <= 0)TempBright = 0;
//						B0 = TempBright;
//						OLED_ShowNum(0,6,B0,3);
//						Serial_Printf("[INFO]Set_LED_0_:%d\r\n",(int)B0);//状态回传上位机
//					}
//					else if (strstr(Serial_RxPacket, "LED1%") != NULL) {
//						int16_t speed;
//						sscanf(Serial_RxPacket, "LED1%%%hd", &TempBright);
//						if (TempBright >= 100)TempBright = 100;
//						if (TempBright <= 0)TempBright = 0;
//						B1 = TempBright;
//						OLED_ShowNum(1,6,B1,3);			
//						Serial_Printf("[INFO]Set_LED_1_:%d\r\n",(int)B1);//状态回传上位机
//					}
//					else if (strstr(Serial_RxPacket, "LED2%") != NULL) {
//						int16_t speed;
//						sscanf(Serial_RxPacket, "LED2%%%hd", &TempBright);
//						if (TempBright >= 100)TempBright = 100;
//						if (TempBright <= 0)TempBright = 0;
//						B2 = TempBright;
//						OLED_ShowNum(2,6,B2,3);
//						Serial_Printf("[INFO]Set_LED_2_:%d\r\n",(int)B2);//状态回传上位机
//					}
//					else if (strstr(Serial_RxPacket, "LED3%") != NULL) {
//						int16_t speed;
//						sscanf(Serial_RxPacket, "LED3%%%hd", &TempBright);
//						if (TempBright >= 100)TempBright = 100;
//						if (TempBright <= 0)TempBright = 0;
//						B3 = TempBright;
//						OLED_ShowNum(3,6,B3,3);
//						Serial_Printf("[INFO]Set_LED_3_:%d\r\n",(int)B3);//状态回传上位机
//					}
//					else {
//						Serial_SendString("[INFO]ERROR_COMMAND\r\n");//状态回传上位机
//					}
					
					//更 简单/宽松 的判定
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
									OLED_ShowNum(1,6,B0,3);
									break;
								
								case 1:
									B1 = TempBright;
									OLED_ShowNum(2,6,B1,3);
									break;
								
								case 2:
									B2 = TempBright;
									OLED_ShowNum(3,6,B2,3);
									break;
								
								case 3:
									B3 = TempBright;
									OLED_ShowNum(4,6,B3,3);
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
	}		
}

//定时器触发时间为0.lms
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2,TIM_IT_Update) == SET )
	{
		LED_Tick();
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}
