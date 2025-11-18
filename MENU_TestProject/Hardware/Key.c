#include "stm32f10x.h"                  // Device header
#include "Delay.h"

volatile uint8_t Key_Num;

void Key_Init(void)//按键初始化
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t Key_GetNum(void)
{
	uint8_t Temp;
	if (Key_Num)//防止可能由中断造成的标志位直接清零，致使按键事件被忽略
	{
		Temp = Key_Num;//利用中间变量实现标志位的返回并清零
		Key_Num = 0;
		return Temp;
	}
	return 0;
}

uint8_t Key_GetState(void)//获取当前按键状态的子函数
{
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 1)
	{
		return 1;
	}
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 1)
	{
		return 2;
	}
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == 1)
	{
		return 3;
	}
	return 0;//没有任何按键按下
}

void Key_Tick(void)//利用定时中断调用，获取通用的时间基准
{
	static uint8_t Count;//定义静态变量
	static uint8_t CurrState, PrevState;//Current,Previous
	//静态变量默认值为0，函数退出后值不会丢失
	
	Count ++;//计数分频
	if (Count >= 20)
	{
		Count = 0;
		
		PrevState = CurrState;
		CurrState = Key_GetState();
		
		if (CurrState == 0 && PrevState != 0)//捕获按键松手瞬间
		{
			Key_Num = PrevState;//键码标志位
		}
	}
}
