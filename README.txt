RS-EC-SecondCheck
RS机器人实验室二轮考核任务

引脚分配见具体项目文件夹

ABC文件直接对应相关项目
D1，E1文件是早期使用BMP库解算MPU6050的版本，未实现快速刷新的解算方法（我本人也不会aaa）
D2，E2文件是后期使用互补滤波解算MPU6050的版本，功能完备
ComplementaryFilter文件是使用互补滤波的模板性文件
MENU_TeatProject文件是考核任务的菜单化整合

2025.10.26
编写了任务A，任务B

2025.10.28
测试并完善了任务A，任务B
创建了此仓库

2025.10.30
完成了任务C

学习参考了B站
【【江协STM32】MPU6050姿态解算】https://www.bilibili.com/video/BV12o5vzwEPp?p=3&vd_source=befff269c2e0ef1583e2528659770521
在任务D的文件夹进行了BMP库的应用

任务D进阶任务未能完成（现D1文件）

编写了任务E初始代码（硬件I2C，BMP库应用）

2025.11.12
学习参考了B站互补滤波解算MPU6050
【平衡车入门教程-持续更新中】https://www.bilibili.com/video/BV1kBanzfExg?p=5&vd_source=befff269c2e0ef1583e2528659770521
利用Copilot完成了D进阶任务（文件D2）
完成了任务E（文件E2）

除了没有使用BMP库（找（抄）不到好用的解算方法），任务程序上基本完成

2025.11
日志未编写

2025.11.19
完成的考核任务（ABCDE）的菜单化
（注：MPU6050仍然使用互补滤波）

”破坏式“（工程重建）地修复了D1的bug