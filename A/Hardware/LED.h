#ifndef __LED_H
#define __LED_H

// 宏定义：LED引脚（PB12~PB15）
#define LED_COUNT				4 
#define LED_PIN_0 GPIO_Pin_0
#define LED_PIN_1 GPIO_Pin_1
#define LED_PIN_2 GPIO_Pin_10
#define LED_PIN_3 GPIO_Pin_11
#define LED_ALL_PINS (LED_PIN_0 | LED_PIN_1 | LED_PIN_2 | LED_PIN_3)

extern uint16_t B0 , B1 , B2 , B3;

void LED_Init(void);
void LED_Tick(void);

#endif
