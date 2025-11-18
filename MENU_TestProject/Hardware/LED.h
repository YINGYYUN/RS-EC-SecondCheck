#ifndef __LED_H
#define __LED_H

extern uint8_t LED_Mode;

/*模式定义*/
#define LED_ALL_OFF_Mode             0
#define LED_ALL_ON_Mode              1
#define LED_SET_BRIGHT_MODE			2
#define LED_SET_FLASH_MODE			3

#define LED_PIN_0_SlowFlash_Mode      10
#define LED_PIN_0_FastFlash_Mode      11 


// LED引脚（PB0,PB1,PB10,PB11）
#define LED_COUNT				4 
#define LED_PIN_0 				GPIO_Pin_2
#define LED_PIN_1 				GPIO_Pin_3
#define LED_PIN_2 				GPIO_Pin_4
#define LED_PIN_3 				GPIO_Pin_5
#define LED_ALL_PINS 			(LED_PIN_0 | LED_PIN_1 | LED_PIN_2 | LED_PIN_3)

//亮度/频率控制
extern uint16_t B0 , B1 , B2 , B3;

void LED_Init(void);
void LED_SetMode(uint8_t Mode);
void LED_Tick(void);

#endif
