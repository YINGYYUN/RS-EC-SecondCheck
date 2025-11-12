#include "stm32f10x.h"                  // Device header
#include <math.h>
#include "MPU6050_Reg.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "Serial.h"

/*引脚配置*/
#define MPU6050_SCL        GPIO_Pin_10    //PB10
#define MPU6050_SDA        GPIO_Pin_11    //PB11

#define MPU6050_ADDRESS        0xD0             //写地址
#define DEFAULT_MPU_HZ         (100)            //定义输出速度
#define q30                    1073741824.0f    //q30格式，long转float时的除数

//陀螺仪方向设置
static signed char gyro_orientation[9] = { 1,  0,  0,
                                           0,  1,  0,
                                           0,  0,  1};
float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
unsigned long sensor_timestamp;
short gyro[3], accel[3], sensors;
unsigned char more;
long quat[4];
										   
/**
  * 函    数：MPU6050等待事件
  * 参    数：同I2C_CheckEvent
  * 返 回 值：无
  */
void MPU6050_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
	uint32_t Timeout;
	Timeout = 10000;									//给定超时计数时间
	while (I2C_CheckEvent(I2Cx, I2C_EVENT) != SUCCESS)	//循环等待指定事件
	{
		Timeout --;										//等待时，计数值自减
		if (Timeout == 0)								//自减到0后，等待超时
		{
			/*超时的错误处理代码，可以添加到此处*/
			break;										//跳出等待，不等了
		}
	}
}										   

/**
  * @brief  MPU6050指定地址写一个字节函数
  * @param  RegAddress 寄存器地址
  * @param  Data 要写入的字节数据
  * @retval 无
  */
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
    I2C_Send7bitAddress(I2C2, MPU6050_ADDRESS, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
    I2C_SendData(I2C2, RegAddress);											//硬件I2C发送寄存器地址
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
	I2C_SendData(I2C2, Data);												//硬件I2C发送数据
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTOP(I2C2, ENABLE);											//硬件I2C生成终止条件
}

/**
  * @brief  MPU6050指定地址读一个字节函数
  * @param  RegAddress 寄存器地址
  * @retval 返回读出的字节数据
  */
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
    uint8_t Data;

    I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
    I2C_Send7bitAddress(I2C2, MPU6050_ADDRESS, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
    I2C_SendData(I2C2, RegAddress);											//硬件I2C发送寄存器地址
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
    I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
    I2C_Send7bitAddress(I2C2, MPU6050_ADDRESS, I2C_Direction_Receiver);		//硬件I2C发送从机地址，方向为接收
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);		//等待EV6
	
	I2C_AcknowledgeConfig(I2C2, DISABLE);									//在接收最后一个字节之前提前将应答失能
	I2C_GenerateSTOP(I2C2, ENABLE);											//在接收最后一个字节之前提前申请停止条件
	
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED);				//等待EV7
	Data = I2C_ReceiveData(I2C2);											//接收数据寄存器
	
	I2C_AcknowledgeConfig(I2C2, ENABLE);									//将应答恢复为使能，为了不影响后续可能产生的读取多字节操作	
	
    return Data;
}

/**
  * @brief  MPU6050指定地址连续写字节函数
  * @param  Addr 器件地址
  * @param  Reg  寄存器地址
  * @param  Len  要写入的数据长度
  * @param  Buf  写入数据的存储区
  * @retval 返回0表示正常，其他表示失败
  */
uint8_t MPU6050_Write_Len(uint8_t Addr, uint8_t Reg, uint8_t Len, uint8_t *Buf)
{
    I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
    I2C_Send7bitAddress(I2C2, Addr << 1, I2C_Direction_Transmitter);		//发送器件地址+写命令
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
    I2C_SendData(I2C2, Reg);												//硬件I2C发送寄存器地址
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
	while(Len--)
	{
		I2C_SendData(I2C2, *Buf++);											//硬件I2C发送数据
		MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);			//等待EV8_2
	}
	
    I2C_GenerateSTOP(I2C2, ENABLE);	
	
	return 0;
} 

/**
  * @brief  MPU6050指定地址连续读字节函数
  * @param  Addr 器件地址
  * @param  Reg  寄存器地址
  * @param  Len  要读取的数据长度
  * @param  Buf  读取数据的存储区
  * @retval 返回0表示正常，其他表示失败
  */
uint8_t MPU6050_Read_Len(uint8_t Addr, uint8_t Reg, uint8_t Len, uint8_t *Buf)
{
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
    I2C_Send7bitAddress(I2C2, Addr << 1, I2C_Direction_Transmitter);		//发送器件地址+写命令
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
    I2C_SendData(I2C2, Reg);												//硬件I2C发送寄存器地址
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C2, Addr << 1, I2C_Direction_Receiver);			//硬件I2C发送从机地址，方向为接收
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);		//等待EV6
	while(Len--)
	{	
		if(Len)																//发送ACK
		{
			I2C_AcknowledgeConfig(I2C2, ENABLE);							//提前产生应答
		}     
		else																//发送nACK
		{
			I2C_AcknowledgeConfig(I2C2, DISABLE);							//在接收最后一个字节之前提前将应答失能
			I2C_GenerateSTOP(I2C2, ENABLE);									//在接收最后一个字节之前提前申请停止条件
		}     	
		MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED);			//等待EV7
		*Buf++ = I2C_ReceiveData(I2C2);										//接收数据寄存器
		
	}
	
	I2C_AcknowledgeConfig(I2C2, ENABLE);									//将应答恢复为使能，为了不影响后续可能产生的读取多字节操作
	
	return 0;
}

/**
* 函    数：MPU6050初始化I2C函数,PB10为I2C_SCL,PB11为I2C_SDA
  * 参    数：无
  * 返 回 值：无
  */
void MPU6050_I2C_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);		//开启I2C2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Pin = MPU6050_SCL | MPU6050_SDA;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PB10和PB11引脚初始化为复用开漏输出
	
	/*I2C初始化*/
	I2C_InitTypeDef I2C_InitStructure;						//定义结构体变量
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;				//模式，选择为I2C模式
	I2C_InitStructure.I2C_ClockSpeed = 50000;				//时钟速度，选择为50KHz
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;		//时钟占空比，选择Tlow/Thigh = 2
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;				//应答，选择使能
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	//应答地址，选择7位，从机模式下才有效
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;				//自身地址，从机模式下才有效
	I2C_Init(I2C2, &I2C_InitStructure);						//将结构体变量交给I2C_Init，配置I2C2
	
	/*I2C使能*/
	I2C_Cmd(I2C2, ENABLE);									//使能I2C2，开始运行
}

/**
  * @brief  MPU6050初始化函数
  * @param  无
  * @retval 无
  */
void MPU6050_Init(void)
{
    MPU6050_I2C_Init();
	
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);      //不复位，解除睡眠，不循环，温度传感器不失能，选择X轴陀螺仪时钟
    MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);      //不循环，不待机
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);      //10分频，采样率1kHz / (1 + 9) = 100Hz
    MPU6050_WriteReg(MPU6050_CONFIG, 0x06);          //无外部同步，数字滤波模式6
    MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);     //陀螺仪不自测，量程±2000°/s
    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);    //加速度计不自测，量程±16g，不使用高通滤波器
}

/**
  * @brief  MPU6050获取ID号函数
  * @param  无
  * @retval 返回ID号
  */
uint8_t MPU6050_GetID(void)
{
    return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

/**
  * @brief  MPU6050获取数据函数
  * @param  AccX 加速度计X轴数据
  * @param  AccY 加速度计Y轴数据
  * @param  AccZ 加速度计Z轴数据
  * @param  GyroX 陀螺仪X轴数据
  * @param  GyroY 陀螺仪Y轴数据
  * @param  GyroZ 陀螺仪Z轴数据
  * @retval 无
  */
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
    uint8_t DataH, DataL;
    
    DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);
    *AccX = (DataH << 8) | DataL;
    DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);
    *AccY = (DataH << 8) | DataL;
    DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);
    *AccZ = (DataH << 8) | DataL;
    DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);
    *GyroX = (DataH << 8) | DataL;
    DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);
    *GyroY = (DataH << 8) | DataL;
    DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);
    *GyroZ = (DataH << 8) | DataL;  
}

/**
  * @brief  DMP初始化函数
  * @param  无
  * @retval 无
  */
void MPU6050_DMPInit(void)
{
	uint8_t res = 0;
    
	MPU6050_I2C_Init();
    res = mpu_init();
    if(!res)
    {
        Serial_Printf("mpu initialization complete ......\r\n");
        
		//设置所需要的传感器
        res = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
        if(!res)
            Serial_Printf("mpu_set_sensor complete ......\r\n");
        else
            Serial_Printf("mpu_set_sensor come across error ......\r\n");
		
        //设置FIFO
        res = mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
        if(!res)
            Serial_Printf("mpu_configure_fifo complete ......\r\n");
        else
            Serial_Printf("mpu_configure_fifo come across error ......\r\n");
        
		//设置采样率
        res = mpu_set_sample_rate(DEFAULT_MPU_HZ);
        if(!res)
            Serial_Printf("mpu_set_sample_rate complete ......\r\n");
        else
            Serial_Printf("mpu_set_sample_rate come across error ......\r\n");
        
		//加载DMP固件
        res = dmp_load_motion_driver_firmware();
        if(!res)
            Serial_Printf("dmp_load_motion_driver_firmware complete ......\r\n");
        else
            Serial_Printf("dmp_load_motion_driver_firmware come across error ......\r\n");
        
		//设置陀螺仪方向
        res = dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));
        if(!res)
            Serial_Printf("dmp_set_orientation complete ......\r\n");
        else
            Serial_Printf("dmp_set_orientation come across error ......\r\n");
        
		//设置DMP功能
        res = dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |	              
              DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
              DMP_FEATURE_GYRO_CAL);
        if(!res)
            Serial_Printf("dmp_enable_feature complete ......\r\n");
        else
            Serial_Printf("dmp_enable_feature come across error ......\r\n");
        
		//设置DMP输出速率(最大不超过200Hz)
        res = dmp_set_fifo_rate(DEFAULT_MPU_HZ);
        if(!res)
            Serial_Printf("dmp_set_fifo_rate complete ......\r\n");
        else
            Serial_Printf("dmp_set_fifo_rate come across error ......\r\n");
        
		//自检
        res = run_self_test();
        if(!res)
            Serial_Printf("mpu_run_self_test complete ......\r\n");
        else
            Serial_Printf("mpu_run_self_test come across error ......\r\n");
        
		//使能DMP
        res = mpu_set_dmp_state(1);
        if(!res)
            Serial_Printf("mpu_set_dmp_state complete ......\r\n");
        else
            Serial_Printf("mpu_set_dmp_state come across error ......\r\n");
    }
    else
    {
        Serial_Printf("mpu initialization come across error ......\r\n");
        while(1);
    }
}

/**
  * @brief  获取DMP处理后数据函数
  * @param  Pitch 俯仰角，精度:0.1°，范围:-90.0° ~ +90.0°
  * @param  Roll  横滚角，精度:0.1°，范围:-180.0°~ +180.0°
  * @param  yaw   偏航角，精度:0.1°，范围:-180.0°~ +180.0°
  * @retval 返回0表示正常，其他表示失败
  */
uint8_t MPU6050_ReadDMP(float *Pitch, float *Roll, float *Yaw)
{	
	if(dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more))  return 1;	
	if(sensors & INV_WXYZ_QUAT)
	{    
        q0 = quat[0] / q30;    //q30格式转换为浮点数
        q1 = quat[1] / q30;
        q2 = quat[2] / q30;
        q3 = quat[3] / q30;
        //计算得到俯仰角、横滚角和偏航角
        *Pitch = asin(-2 * q1 * q3 + 2 * q0 * q2) * 57.3; 	
        *Roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 57.3;
        *Yaw   = atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 57.3;
    }else  return 2;
    return 0;
}
