#include "debug_task.h"
#include "Motor.hpp"
#include "topics.hpp"

#include "task.h"

#include "com_config.h"
#include "pid_controller.h"

osThreadId_t Debug_TaskHandle;

extern C620Motor arm3508_motor;

// pid结构体
PID_t motor_pid1 = {
    .Kp = 1.0, .Ki = 0.0, .Kd = 0.0, .MaxOut = 20000, .Improve = NONE};
PID_t motor_pid2 = {
    .Kp = 1.0, .Ki = 0.0, .Kd = 0.0, .MaxOut = 20000, .Improve = NONE};

float target_speed = 0.0f;
float output = 0.0f;

static inline void debugInit(void) {
  // pid初始化
  PID_Init(&motor_pid1);
  PID_Init(&motor_pid2);
}

void debugTask(void *argument) {
  TickType_t currentTime;
  currentTime = xTaskGetTickCount();
  debugInit();
  for (;;) {
    output = PID_Calculate(&motor_pid1, arm3508_motor.getCurrentSpeed(),
                           target_speed);
    arm3508_motor.setMotorCmd(output);

    vTaskDelayUntil(&currentTime, 5);
  }
}