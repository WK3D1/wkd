# 全自动哨兵机器人 · 嵌入式工程

RoboMaster 机甲大师赛全自动哨兵机器人嵌入式软件。基于 STM32F407 + FreeRTOS，三块主控板（主控 / 左 / 右）通过 CAN 总线协同，实现全向底盘运动、两级云台自瞄、双发射机构控制、裁判系统交互与上位机通信。

## 工程结构

- `New_Sentry/` 主控工程
- `New_Sentry_Left/` 左板工程
- `New_Sentry_Right/` 右板工程

每个工程内，自研代码位于 `MDK-ARM/` 下的功能模块目录：

- `ALG/` Kalman、PID、通用数学库
- `Chassis/` 底盘运动控制（麦轮全向运动学解算）
- `Gimbal/` 云台串级 PID 控制
- `IMU/` BMI088 陀螺仪 / IST8310 磁力计采集与 Mahony 姿态解算
- `Motor/` DJI3508、DM4310 电机驱动与封装
- `Judge/` 裁判系统串口协议解析与客户端 UI 绘制
- `Communication/` CAN 板间通信（裁判 / 全局 / 遥控 / 视觉）
- `DBUS/` 遥控器协议
- `Vision/` 视觉通信
- `Protect/` 安全保护
- `Shoot/` 发射机构控制

## 关键实现

- 电机控制：前馈 + 串级积分分离 PID，通过 CAN 对驱动电机做速度环 / 位置环闭环
- 姿态解算：BMI088（SPI）六轴数据 + Mahony 算法融合四元数，解算云台姿态实现自瞄
- 底盘运动：四轮麦轮运动学逆解 + 坐标系变换，实现全向移动
- 通信：UART + DMA + IDLE 中断收变长帧，环形缓冲 + 状态机解析裁判系统与上位机数据
- 滤波：IIR 低通抑制齿轮间隙引起的角速度高频抖动

## 说明

- 底层库（STM32 HAL、FreeRTOS、USB 中间件、CMSIS）由 STM32CubeMX 生成，未随仓库提交；使用 `New_Sentry.ioc` 重新生成即可。
- 比赛成绩：全国三等奖、区域赛一等奖、联盟赛三等奖。
