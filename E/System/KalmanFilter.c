#include "KalmanFilter.h"
#include <math.h>  // 用于atan2f、sqrtf等数学函数

/**
 * 初始化卡尔曼滤波器参数和状态
 */
void KalmanFilter_Init(KalmanFilterTypeDef *kf, float Q_angle, float Q_bias, float R_measure) {
    // 初始化噪声参数
    kf->Q_angle = Q_angle;
    kf->Q_bias = Q_bias;
    kf->R_measure = R_measure;
    
    // 初始化状态变量（角度和偏差初始化为0）
    kf->angle = 0.0f;
    kf->bias = 0.0f;
    
    // 初始化误差协方差矩阵（初始值设为0，表示初始状态确定）
    kf->P[0][0] = 0.0f;
    kf->P[0][1] = 0.0f;
    kf->P[1][0] = 0.0f;
    kf->P[1][1] = 0.0f;
}

/**
 * 卡尔曼滤波更新步骤（预测+校正）
 */
float KalmanFilter_Update(KalmanFilterTypeDef *kf, float angle_measured, float rate_measured, float dt) {
    // 1. 预测步骤（根据运动模型预测当前状态）
    // 修正后的角速度 = 测量角速度 - 偏差
    float rate = rate_measured - kf->bias;
    // 预测角度 = 上一时刻角度 + 修正后的角速度 * 采样时间
    kf->angle += dt * rate;
    
    // 更新误差协方差矩阵（预测）
    kf->P[0][0] += dt * (dt * kf->P[1][1] - kf->P[0][1] - kf->P[1][0] + kf->Q_angle);
    kf->P[0][1] -= dt * kf->P[1][1];
    kf->P[1][0] -= dt * kf->P[1][1];
    kf->P[1][1] += kf->Q_bias * dt;
    
    // 2. 校正步骤（融合测量值修正预测结果）
    // 计算卡尔曼增益
    float S = kf->P[0][0] + kf->R_measure;  // 残差协方差
    float K[2];                             // 卡尔曼增益
    K[0] = kf->P[0][0] / S;
    K[1] = kf->P[1][0] / S;
    
    // 计算残差（测量值与预测值的差）
    float y = angle_measured - kf->angle;
    
    // 更新状态变量（角度和偏差）
    kf->angle += K[0] * y;
    kf->bias += K[1] * y;
    
    // 更新误差协方差矩阵（校正）
    float P00_temp = kf->P[0][0];
    float P01_temp = kf->P[0][1];
    kf->P[0][0] = P00_temp - K[0] * P00_temp;
    kf->P[0][1] = P01_temp - K[0] * P01_temp;
    kf->P[1][0] = kf->P[1][0] - K[1] * P00_temp;
    kf->P[1][1] = kf->P[1][1] - K[1] * P01_temp;
    
    return kf->angle;  // 返回滤波后的角度
}

/**
 * 从加速度计数据计算俯仰角（Pitch）
 * 原理：通过重力加速度在各轴的分量计算角度
 */
float KalmanFilter_GetAccelAngle(int16_t AccX, int16_t AccY, int16_t AccZ) {
    // 俯仰角（绕X轴旋转）：利用Y轴和Z轴的加速度分量计算
    // 公式：atan2(AccY, sqrt(AccX² + AccZ²))
    float pitch = atan2f((float)AccY, sqrtf((float)AccX*(float)AccX + (float)AccZ*(float)AccZ));
    return pitch;
}
