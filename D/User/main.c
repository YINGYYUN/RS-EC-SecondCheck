#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "Serial.h"
#include "Timer.h"

float Pitch, Roll, Yaw;

int main(void)
{
    OLED_Init();
    Serial_Init();
	MPU6050_DMPInit();
	Timer_Init();
	
	OLED_ShowString(0, 0, "MPU6050", OLED_8X16);
	
    while(1)
    {		
		OLED_Printf(0, 16, OLED_8X16, "Roll :%+06.1f", Roll);
		OLED_Printf(0, 32, OLED_8X16, "Yaw  :%+06.1f", Yaw);
		OLED_Printf(0, 48, OLED_8X16, "Pitch:%+06.1f", Pitch);
		
		OLED_Update();
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
