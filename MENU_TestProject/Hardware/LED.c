#include "stm32f10x.h"                  // Device header

#include "LED.h"

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 |GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA, LED_ALL_PINS);
}


/* =========[START] LED开关 [START] ========= */
void LED_0_ON(void)
{
	GPIO_ResetBits(GPIOA,LED_PIN_0);
}
void LED_0_OFF(void)
{
	GPIO_SetBits(GPIOA,LED_PIN_0);
}
void LED_1_ON(void)
{
	GPIO_ResetBits(GPIOA,LED_PIN_1);
}
void LED_1_OFF(void)
{
	GPIO_SetBits(GPIOA,LED_PIN_1);
}
void LED_2_ON(void)
{
	GPIO_ResetBits(GPIOA,LED_PIN_2);
}
void LED_2_OFF(void)
{
	GPIO_SetBits(GPIOA,LED_PIN_2);
}
void LED_3_ON(void)
{
	GPIO_ResetBits(GPIOA,LED_PIN_3);
}
void LED_3_OFF(void)
{
	GPIO_SetBits(GPIOA,LED_PIN_3);
}
void LED_ALL_OFF(void)
{
	GPIO_SetBits(GPIOA,LED_ALL_PINS);
}
void LED_ALL_ON(void)
{
	GPIO_ResetBits(GPIOA,LED_ALL_PINS);
}
/* =========[END] LED开关 [END] ========= */


uint8_t LED_Mode = LED_ALL_OFF_Mode;
static uint16_t LED_TimeCount = 0;
uint16_t B0 = 0, B1 = 0, B2 = 0, B3 = 0;

void LED_SetMode(uint8_t Mode)
{
	LED_Mode = Mode;
	LED_TimeCount = 0;
}


void LED_Tick(void)
{
	
	
	switch(LED_Mode)
	{
		case LED_ALL_OFF_Mode:
			LED_ALL_OFF();
		
			break;
		
		case LED_ALL_ON_Mode:
			LED_ALL_ON();
		
			break;
		
		//LED亮度控制模式(通过串口)
		case LED_SET_BRIGHT_MODE:
			LED_TimeCount ++;
			LED_TimeCount %=10;
			
			if(B0 > LED_TimeCount)LED_0_ON();
			else LED_0_OFF();
			
			if(B1 > LED_TimeCount)LED_1_ON();
			else LED_1_OFF();
			
			if(B2 > LED_TimeCount)LED_2_ON();
			else LED_2_OFF();
			
			if(B3 > LED_TimeCount)LED_3_ON();
			else LED_3_OFF();
		
			break;
		
		//LED闪烁频率控制模式(通过串口)
		case LED_SET_FLASH_MODE:
			LED_TimeCount ++;
			LED_TimeCount %=1000;//防越界
			
			if(B0 > LED_TimeCount)LED_0_ON();
			else LED_0_OFF();
			
			if(B1 > LED_TimeCount)LED_1_ON();
			else LED_1_OFF();
			
			if(B2 > LED_TimeCount)LED_2_ON();
			else LED_2_OFF();
			
			if(B3 > LED_TimeCount)LED_3_ON();
			else LED_3_OFF();
		
			break;
			
		//LED0慢闪
		case LED_PIN_0_SlowFlash_Mode:
			LED_TimeCount ++;
			LED_TimeCount %= 1000;
			if (LED_TimeCount < 100)	{LED_0_ON();}
			else 				{LED_0_OFF();}
			
			break;
		
		//LED0快闪
		case LED_PIN_0_FastFlash_Mode:
			LED_TimeCount ++;
			LED_TimeCount %= 100;
			if (LED_TimeCount < 50)	{LED_0_ON();}
			else 				{LED_0_OFF();}

			break;
		
		default:
			//留一手
		
			break;		
	}
}
