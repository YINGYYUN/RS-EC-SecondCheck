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

	OLED_ShowString(1,1,"N0:");	
	OLED_ShowString(2,1,"N1:");
	OLED_ShowString(3,1,"N2:");
	OLED_ShowString(4,1,"N3:");
	
	OLED_ShowString(1,9,"F0:");	
	OLED_ShowString(2,9,"F1:");
	OLED_ShowString(3,9,"F2:");
	OLED_ShowString(4,9,"F3:");
		
	OLED_ShowNum(1,4,B0,4);
	OLED_ShowNum(2,4,B1,4);
	OLED_ShowNum(3,4,B2,4);
	OLED_ShowNum(4,4,B3,4);
	
	OLED_ShowNum(1,13,1000-B0,4);
	OLED_ShowNum(2,13,1000-B1,4);
	OLED_ShowNum(3,13,1000-B2,4);
	OLED_ShowNum(4,13,1000-B3,4);
	
	Serial_SendString("SYSTEM_READY\r\n");
	
	Serial_RxFlag = 0;
	//查询接收，演示用，不封装
	while(1)
	{
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
									OLED_ShowNum(1,4,B0,4);								
									OLED_ShowNum(1,13,1000-B0,4);
									break;
								
								case 1:
									B1 = TempBright;
									OLED_ShowNum(2,4,B1,4);								
									OLED_ShowNum(2,13,1000-B1,4);
									break;
								
								case 2:
									B2 = TempBright;
									OLED_ShowNum(3,4,B2,4);								
									OLED_ShowNum(3,13,1000-B2,4);
									break;
								
								case 3:
									B3 = TempBright;
									OLED_ShowNum(4,4,B3,4);								
									OLED_ShowNum(4,13,1000-B3,4);
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
	}		
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2,TIM_IT_Update) == SET )
	{
		LED_Tick();
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}
