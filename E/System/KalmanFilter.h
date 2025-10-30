#ifndef __KALMAN_FILTER_H
#define __KALMAN_FILTER_H

#include "stm32f10x.h"  // 依赖STM32基础类型定义

// 卡尔曼滤波器结构体，存储滤波状态和参数
typedef struct {
    float Q_angle;       // 角度测量噪声方差（过程噪声）
    float Q_bias;        // 角速度偏差噪声方差（过程噪声）
    float R_measure;     // 测量噪声方差（观测噪声）
    float angle;         // 滤波后的角度（弧度）
    float bias;          // 角速度偏差（弧度/秒）
    float P[2][2];       // 误差协方差矩阵
} KalmanFilterTypeDef;

/**
 * 初始化卡尔曼滤波器
 * @param kf：卡尔曼滤波器结构体指针
 * @param Q_angle：角度过程噪声方差（建议初始值：0.001）
 * @param Q_bias：角速度偏差过程噪声方差（建议初始值：0.003）
 * @param R_measure：测量噪声方差（建议初始值：0.03）
 */
void KalmanFilter_Init(KalmanFilterTypeDef *kf, float Q_angle, float Q_bias, float R_measure);

/**
 * 更新卡尔曼滤波器（核心滤波逻辑）
 * @param kf：卡尔曼滤波器结构体指针
 * @param angle_measured：加速度计测量的角度（弧度）
 * @param rate_measured：陀螺仪测量的角速度（弧度/秒）
 * @param dt：采样时间间隔（秒，例如10ms则为0.01f）
 * @return 滤波后的角度（弧度）
 */
float KalmanFilter_Update(KalmanFilterTypeDef *kf, float angle_measured, float rate_measured, float dt);

/**
 * 从加速度计原始数据计算角度（俯仰角/Pitch）
 * @param AccX：加速度计X轴原始数据（16位整数）
 * @param AccY：加速度计Y轴原始数据（16位整数）
 * @param AccZ：加速度计Z轴原始数据（16位整数）
 * @return 计算得到的角度（弧度）
 */
float KalmanFilter_GetAccelAngle(int16_t AccX, int16_t AccY, int16_t AccZ);

#endif  // __KALMAN_FILTER_H
