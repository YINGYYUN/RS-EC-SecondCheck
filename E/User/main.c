#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "OLED.h"
#include "MPU6050.h"
#include "Serial.h"
#include "Timer.h"
#include "Servo.h"

volatile float Pitch, Roll, Yaw;
float Angle = 90.0f;
int main(void)
{
	LED_Init();
    OLED_Init();
    Serial_Init();
	MPU6050_DMPInit();
	Serial_Init();
	Servo_Init();
	
	Timer_Init();
	
	LED_SetMode(LED_SlowFlashMode);
	Serial_SendString("[INFO]LED_SlowFlashMode\r\n");
	
	OLED_ShowString(0, 0, "MPU6050", OLED_8X16);
	
	Serial_SendString("[INFO]READY\r\n");
	
    while(1)
    {
		OLED_Printf(0, 16, OLED_8X16, "Pitch:%+06.1f", Pitch);
		OLED_Printf(0, 32, OLED_8X16, "Roll :%+06.1f", Roll);
		OLED_Printf(0, 48, OLED_8X16, "Yaw  :%+06.1f", Yaw);

		OLED_Update();
		  Angle = (Yaw + 180.0f) * 180.0f / 360.0f;

		  /* Angle is a float; use float format. Using %lf with a non-double
			  argument leads to undefined behaviour and the large garbage values
			  observed on the serial port. */
		  Serial_Printf("Angle:%+06.1f\r\n", Angle);
		
		if ((Angle <= 10.0f || 170.0f <= Angle) && LED_Mode == LED_SlowFlashMode )
		{
			LED_SetMode(LED_FastFlashMode);
			Serial_SendString("[INFO]LED_SlowFlashMode\r\n");
		}
		else if((10.0f <= Angle && Angle<= 170.0f) && LED_Mode == LED_FastFlashMode )
		{
			LED_SetMode(LED_SlowFlashMode);
			Serial_SendString("[INFO]LED_FastFlashMode\r\n");			
		}
			
			
			
			Servo_SetAngle(Angle);
				
    }
}


void TIM1_UP_IRQHandler(void)
{
	//检查标志位
	if (TIM_GetITStatus(TIM1,TIM_IT_Update) == SET )
	{
		LED_Tick();
		MPU6050_ReadDMP(&Pitch, &Roll, &Yaw);
		//清除标志位
		TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
	}
}
