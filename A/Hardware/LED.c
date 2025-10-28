#include "stm32f10x.h"                  // Device header

#include "LED.h"

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10 |GPIO_Pin_11 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, LED_ALL_PINS);
}

//LED开关
void LED_0_ON(void)
{
	GPIO_ResetBits(GPIOB,LED_PIN_0);
}
void LED_0_OFF(void)
{
	GPIO_SetBits(GPIOB,LED_PIN_0);
}

void LED_1_ON(void)
{
	GPIO_ResetBits(GPIOB,LED_PIN_1);
}
void LED_1_OFF(void)
{
	GPIO_SetBits(GPIOB,LED_PIN_1);
}

void LED_2_ON(void)
{
	GPIO_ResetBits(GPIOB,LED_PIN_2);
}
void LED_2_OFF(void)
{
	GPIO_SetBits(GPIOB,LED_PIN_2);
}

void LED_3_ON(void)
{
	GPIO_ResetBits(GPIOB,LED_PIN_3);
}
void LED_3_OFF(void)
{
	GPIO_SetBits(GPIOB,LED_PIN_3);
}

static uint16_t LED_TimeCount = 0;//计时
uint16_t B0 = 0, B1 = 0, B2 = 0, B3 = 0;

//LED亮度调节

//执行
void LED_Tick(void)
{
	LED_TimeCount ++;
	LED_TimeCount %=200;//防越界
	
	if(B0 >= LED_TimeCount)LED_0_ON();
	else LED_0_OFF();
	
	if(B1 >= LED_TimeCount)LED_1_ON();
	else LED_1_OFF();
	
	if(B2 >= LED_TimeCount)LED_2_ON();
	else LED_2_OFF();
	
	if(B3 >= LED_TimeCount)LED_3_ON();
	else LED_3_OFF();
	
	if(LED_TimeCount >= 100)
	{
		LED_TimeCount = 0;
	}
}
