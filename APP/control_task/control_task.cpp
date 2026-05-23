/**
 * @file control_task.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 控制中枢，遥控/上位机 接口都接入到此，然后向其他任务发布可能的控制指令
 * @version 0.1
 * @date 2026-04-18
 *
 * @copyright Copyright (c) 2026
 *
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "control_task.h"
#include "topic_pool.h"
#include "topics.hpp"

#include <cstdint>

osThreadId_t ControlTaskHandle;

// 发布者：发送底盘控制命令 (静态初始化，避免动态内存分配)
static TypedTopicPublisher<pub_chassis_cmd> g_chassis_cmd_pub("chassis_cmd");

void controlInit() {
  // 检查发布者初始化是否成功
  if (!g_chassis_cmd_pub.IsValid()) {
    // 初始化失败处理
    return;
  }
}

pub_chassis_cmd pub_cmd{};

void controlTask(void *argument) {
  TickType_t currentTime = xTaskGetTickCount();

  controlInit();

  for (;;) {
    // 示例：构造底盘控制命令并发布
    if (g_chassis_cmd_pub.IsValid()) {
      g_chassis_cmd_pub.Publish(pub_cmd);
    }

    vTaskDelayUntil(&currentTime, 5); // 5ms 执行一次
  }
}