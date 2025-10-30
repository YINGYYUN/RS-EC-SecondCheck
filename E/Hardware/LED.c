#include "stm32f10x.h"                  // Device header
#include "LED.h"

/*引脚配置*/
#define LED1         GPIO_Pin_0    //PA0

/* 持久化计数器，避免未初始化导致的随机闪烁 */
static uint16_t LED_Count = 0;

/**
	* @brief  LED初始化函数，初始化PA0为LED输出
	* @param  无
	* @retval 无
	*/
void LED_Init(void)
{
		//配置RCC
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
		//配置GPIO
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽输出模式
		GPIO_InitStructure.GPIO_Pin = LED1;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
    
		GPIO_SetBits(GPIOA, LED1);    //默认熄灭（视电路为高电平熄灭）
}

void LED_ON(void)
{
		GPIO_SetBits(GPIOA, LED1);
}

void LED_OFF(void)
{
		GPIO_ResetBits(GPIOA, LED1);
}

uint8_t LED_Mode = LED_SlowFlashMode;

void LED_SetMode(uint8_t Mode)
{
		if (Mode == LED_SlowFlashMode || Mode == LED_FastFlashMode)
		{
				LED_Mode = Mode;
		}
}

/**
	* @brief  LED定时中断处理，由1ms定时器或其他定时调用
	*/
void LED_Tick(void)
{
		switch (LED_Mode)
		{
				case LED_SlowFlashMode:
					LED_Count++;
					LED_Count %= 1000;    // 周期 1000 ms
					if (LED_Count < 500)  { LED_ON(); }   // 500ms ON / 500ms OFF
					else                  { LED_OFF(); }
					break;

				case LED_FastFlashMode:
					LED_Count++;
					LED_Count %= 100;     // 周期 100 ms
					if (LED_Count < 50)   { LED_ON(); }   // 50ms ON / 50ms OFF
					else                  { LED_OFF(); }
					break;

				default:
						/* 保持当前状态 */
						break;
		}
}
