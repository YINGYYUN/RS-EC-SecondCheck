#include "stm32f10x.h"                  // Device header
#include "PWM.h"

/**
  * 函    数：舵机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Servo_Init(void)
{
	PWM_Init();									//初始化舵机的底层PWM
}

/**
  * 函    数：舵机设置角度
  * 参    数：Angle 要设置的舵机角度，范围：0~180
  * 返 回 值：无
  */
void Servo_SetAngle(float Angle)
{
  // 将角度映射到常见舵机的 1.0ms ~ 2.0ms 脉宽范围（在 20ms 周期内）
  // TIM2 的定时器周期为 20000 (20ms)，计数频率为 1MHz 时，1ms = 1000
  float pulse = Angle / 180.0f * 1000.0f + 1000.0f; // [1000,2000]
  if (pulse < 1000.0f) pulse = 1000.0f;
  if (pulse > 2000.0f) pulse = 2000.0f;
  PWM_SetCompare2((uint16_t)pulse);
}
