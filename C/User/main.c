#include "stm32f10x.h"  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"

uint8_t ID;								//定义用于存放ID号的变量
int16_t AX, AY, AZ, GX, GY, GZ;			//定义用于存放各个数据的变量

int main(void)
{	
	OLED_Init();
	MPU6050_Init();

	OLED_ShowString(0, 0, "ID:",OLED_8X16);
	ID = MPU6050_GetID();	
	OLED_ShowHexNum(24, 0, ID, 2, OLED_8X16);
	
	OLED_Update();
	
	while(1)
	{		
		//获取MPU6050的数据
		MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);

		float AX_g, AY_g, AZ_g;
		float GX_dps, GY_dps, GZ_dps;

		AX_g = AX / 32768.0 * 16 * 9.8;
		AY_g = AY / 32768.0 * 16 * 9.8;
		AZ_g = AZ / 32768.0 * 16 * 9.8;
		
		GX_dps = GX / 32768.0 * 2000;
		GY_dps = GY / 32768.0 * 2000;
		GZ_dps = GZ / 32768.0 * 2000;
		
		
		
		OLED_Printf(0, 16, OLED_8X16, "%+05.2f", AX_g);					//OLED显示数据
		OLED_Printf(0, 32, OLED_8X16, "%+05.2f", AY_g);
		OLED_Printf(0, 48, OLED_8X16, "%+05.2f", AZ_g);
		OLED_Printf(57, 16, OLED_8X16, "%+05.2f", GX_dps);
		OLED_Printf(57, 32, OLED_8X16, "%+05.2f", GY_dps);
		OLED_Printf(57, 48, OLED_8X16, "%+05.2f", GZ_dps);

		OLED_Update();
	}
		
}
