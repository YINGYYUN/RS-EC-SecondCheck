#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "OLED.h"
#include "MPU6050.h"
#include "Serial.h"
#include "Timer.h"
#include "Servo.h"

float Pitch, Roll, Yaw;
uint16_t Angle = 90 ;
int main(void)
{
	LED_Init();
    OLED_Init();
    Serial_Init();
	MPU6050_DMPInit();
	Servo_Init();
	
	Timer_Init();
	
	LED_SetMode(LED_SlowFlashMode);
	
	OLED_ShowString(0, 0, "MPU6050", OLED_8X16);
	
    while(1)
    {
		OLED_Printf(0, 16, OLED_8X16, "Pitch:%+06.1f", Pitch);
		OLED_Printf(0, 32, OLED_8X16, "Roll :%+06.1f", Roll);
		OLED_Printf(0, 48, OLED_8X16, "Yaw  :%+06.1f", Yaw);
		
		OLED_Update();
		
		Angle = (Yaw + 180.0f) * 180.0f / 360.0f;
		if(Angle <= 10.0 || 170.0<= Angle)	{LED_SetMode(LED_SlowFlashMode);}
		else 							{LED_SetMode(LED_FastFlashMode);}
		Servo_SetAngle(Angle);
		
		
    }
}


void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)    //判断是否为定时器TIM2更新中断
	{
		MPU6050_ReadDMP(&Pitch, &Roll, &Yaw);
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);    //清除定时器TIM2更新中断标志位 
	}
}
