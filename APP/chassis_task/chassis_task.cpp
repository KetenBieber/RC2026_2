/**
 * @file chassis_task.cpp
 * @author Keten (2863861004@qq.com)
 * @brief
 * @version 0.1
 * @date 2026-04-18
 *
 * @copyright Copyright (c) 2026
 *
 * @attention :
 * @note :
 * @versioninfo :
 */

#include "chassis_task.h"
#include "Motor.hpp"
#include "com_config.h"
#include "topic_pool.h"
#include "topics.hpp"

#include <cstdint>
#include <cstring>

osThreadId_t ChassisTaskHandle;

// extern C620Motor chassis_motor1, chassis_motor2, chassis_motor3,
// chassis_motor4;

// 订阅者：接收底盘控制命令 (静态初始化，避免动态内存分配)
static TypedTopicSubscriber<pub_chassis_cmd> g_chassis_cmd_sub("chassis_cmd",
                                                               8);

// 用到的初始化
static inline void chassisInit() {
  // PID初始化
  // pid1.Init(...);
  // pid2.Init(...);

  // 检查订阅者初始化是否成功
  if (!g_chassis_cmd_sub.IsValid()) {
    // 订阅失败处理
    return;
  }
}

static inline void cmdCalculate(const pub_chassis_cmd &cmd) {
  // 根据控制命令计算电机速度
  //
  // 例如：Mecanum 底盘 4 电机速度计算
  // motor_speed[0] = cmd.linear_x_ + cmd.linear_y_ + cmd.omega_;
  // motor_speed[1] = cmd.linear_x_ - cmd.linear_y_ + cmd.omega_;
  // motor_speed[2] = cmd.linear_x_ - cmd.linear_y_ - cmd.omega_;
  // motor_speed[3] = cmd.linear_x_ + cmd.linear_y_ - cmd.omega_;

  (void)cmd; // 消除未使用的参数警告
}

pub_chassis_cmd cmd{};

void chassisTask(void *argument) {
  TickType_t currentTime = xTaskGetTickCount();

  chassisInit();
  int i = 0;

  for (;;) {
    // 尝试从 "chassis_cmd" 主题获取最新的控制命令

    if (g_chassis_cmd_sub.TryGet(&cmd)) {
      // 成功获取新的控制命令
      cmdCalculate(cmd);

      if (i == 8) {
        i = 0;
      }
      // 应用电机速度
      // chassis_motor1.setSpeed(motor_speed[0]);
      // chassis_motor2.setSpeed(motor_speed[1]);
      // chassis_motor3.setSpeed(motor_speed[2]);
      // chassis_motor4.setSpeed(motor_speed[3]);
    }
    // 如果没有新消息，保持上一次的电机速度

    vTaskDelayUntil(&currentTime, 5); // 每5ms执行一次发送任务
  }
}
