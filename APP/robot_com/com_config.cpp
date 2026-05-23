#include "com_config.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "portmacro.h"
#include "stm32h7xx_hal_uart.h"
#include "task.h"

#include "Canbus.hpp"
#include "Motor.hpp"
#include "ROSCom.hpp"
#include "UartPort.hpp"
#include "UsbPort.hpp"
#include "XboxRemote.hpp"
#include "usart.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

osThreadId_t CAN1_Send_TaskHandle;
osThreadId_t CAN2_Send_TaskHandle;
osThreadId_t CAN3_Send_TaskHandle;
osThreadId_t uart3ProcessTaskHandle;
osThreadId_t usbcdcProcessTaskHandle;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;
extern FDCAN_HandleTypeDef hfdcan3;

CanBus fdcan1_bus(hfdcan1);
CanBus fdcan2_bus(hfdcan2);
CanBus fdcan3_bus(hfdcan3);

// can设备

// 底盘电机
// C620Motor chassis_motor1(&fdcan2_bus, 0x201, 0, 0x200, 0);
// C620Motor chassis_motor2(&fdcan2_bus, 0x202, 0, 0x200, 0);
// C620Motor chassis_motor3(&fdcan2_bus, 0x203, 0, 0x200, 0);
// C620Motor chassis_motor4(&fdcan2_bus, 0x204, 0, 0x200, 0);

//
C610Motor arm2006_motor(&fdcan2_bus, 0x201, 0, 0x200, 0);
C620Motor arm3508_motor(&fdcan2_bus, 0x203, 0, 0x200, 0);

// 串口外设（回调+信号量唤醒处理线程进行解包）
void onUart3RxCb(const uint8_t *data, size_t len, void *user);

void onUsbRxCb(const uint8_t *data, size_t len, void *user);

extern UART_HandleTypeDef huart3;

DMA_BUFFER_ATTR static uint8_t uart3_rx_dma[64];
DMA_BUFFER_ATTR static uint8_t uart3_tx_dma[64];
UartPort uart3_port(&huart3, uart3_rx_dma, sizeof(uart3_rx_dma), uart3_tx_dma,
                    sizeof(uart3_tx_dma), onUart3RxCb, nullptr);
osSemaphoreId_t uart3_rx_semphore = NULL;

// Xbox控制器（基于uart3）
XboxRemote xbox_remote(uart3_port);

// usb
osSemaphoreId_t usbcdc_rx_semphore = NULL;
ROSProtocol ros_protocol(nullptr, &UsbPort::Instance());

uint8_t comServiceInit() {
  // can外设初始化
  canFilterInit(&hfdcan1, FDCAN_STANDARD_ID, FDCAN_FILTER_TO_RXFIFO0, 0, 0);
  canFilterInit(&hfdcan1, FDCAN_STANDARD_ID, FDCAN_FILTER_TO_RXFIFO1, 0, 0);
  bspCanInit(&hfdcan1);
  canFilterInit(&hfdcan2, FDCAN_STANDARD_ID, FDCAN_FILTER_TO_RXFIFO0, 0, 0);
  canFilterInit(&hfdcan2, FDCAN_STANDARD_ID, FDCAN_FILTER_TO_RXFIFO1, 0, 0);
  bspCanInit(&hfdcan2);
  canFilterInit(&hfdcan3, FDCAN_STANDARD_ID, FDCAN_FILTER_TO_RXFIFO0, 0, 0);
  canFilterInit(&hfdcan3, FDCAN_STANDARD_ID, FDCAN_FILTER_TO_RXFIFO1, 0, 0);
  bspCanInit(&hfdcan3);

  // can 总线初始化
  fdcan1_bus.init();
  fdcan2_bus.init();
  fdcan3_bus.init();

  // chassis_motor1.init();
  // chassis_motor2.init();
  // chassis_motor3.init();
  // chassis_motor4.init();

  arm2006_motor.init();
  arm3508_motor.init();

  // fdcan2_bus.registerDevice(&chassis_motor1);
  // fdcan2_bus.registerDevice(&chassis_motor2);
  // fdcan2_bus.registerDevice(&chassis_motor3);
  // fdcan2_bus.registerDevice(&chassis_motor4);
  fdcan2_bus.registerDevice(&arm2006_motor);
  fdcan2_bus.registerDevice(&arm3508_motor);

  // 串口外设
  uart3_rx_semphore = osSemaphoreNew(1, 0, NULL);
  uart3_port.startRxDmaIdle();

  // Xbox控制器初始化
  xbox_remote.init();

  // usb 外设
  usbcdc_rx_semphore = osSemaphoreNew(1, 0, NULL);
  ros_protocol.init();
  UsbPort::Instance().SetRxCallback(onUsbRxCb, NULL);

  return 0;
}

void onUart3RxCb(const uint8_t *data, size_t len, void *user) {
  (void)user;
  if (data != nullptr && len > 0 && uart3_rx_semphore != NULL) {
    (void)osSemaphoreRelease(uart3_rx_semphore);
  }
}

void onUsbRxCb(const uint8_t *data, size_t len, void *user) {
  (void)user;
  if (data != nullptr && len > 0 && usbcdc_rx_semphore != NULL) {
    (void)osSemaphoreRelease(usbcdc_rx_semphore);
  }
}

void can1SendTask(void *argument) {
  TickType_t currentTime = xTaskGetTickCount();

  for (;;) {

    vTaskDelayUntil(&currentTime, 1); // 每1ms执行一次发送任务
  }
}

void can2SendTask(void *argument) {
  TickType_t currentTime = xTaskGetTickCount();
  CanBus::ClassicPack pack;
  pack.type = CanBus::Type::STANDARD;
  pack.id = 0x200; // DJI Group 2

  uint8_t len = 8;
  // const uint32_t motor_ids[4] = {0x201, 0x202, 0x203, 0x204};
  for (;;) {
    // 一帧固定打包 4 个槽位：0x201~0x204
    // 当前仅有 0x201(arm2006) 和 0x203(arm3508)，其余槽位置 0
    // int16_t commands[4] = {0};
    // commands[0] = static_cast<int16_t>(arm2006_motor.cmdTrans()); // 0x201
    // commands[2] = static_cast<int16_t>(arm3508_motor.cmdTrans()); // 0x203

    pack.data[4] = static_cast<uint8_t>(
        static_cast<int16_t>(arm3508_motor.cmdTrans()) >> 8);
    pack.data[5] =
        static_cast<uint8_t>(static_cast<int16_t>(arm3508_motor.cmdTrans()));
    pack.data[6] = static_cast<uint8_t>(
        static_cast<int16_t>(arm2006_motor.cmdTrans()) >> 8);
    pack.data[7] =
        static_cast<uint8_t>(static_cast<int16_t>(arm2006_motor.cmdTrans()));

    // packDJIMotorCanMsg(pack.id, motor_ids, commands, 4, pack.data, len);
    // arm3508_motor.manager_->addCanMsg(pack);
    fdcan2_bus.addCanMsg(pack);

    vTaskDelayUntil(&currentTime, 1); // 每1ms执行一次发送任务
  }
}

void can3SendTask(void *argument) {
  TickType_t currentTime = xTaskGetTickCount();
  for (;;) {

    vTaskDelayUntil(&currentTime, 1); // 每1ms执行一次发送任务
  }
}

// 根据不同的操作系统封装
// 你windows下使用thread库，它
// Linux使用thread

int a = 0;

void uart3RxProcessTask(void *argument) {
  (void)argument;
  for (;;) {
    if (uart3_rx_semphore == NULL) {
      a++;
    }
    (void)osSemaphoreAcquire(uart3_rx_semphore, osWaitForever);

    UartPort::Packet packet{};
    while (uart3_port.Read(packet)) {
      // 逐字节送进Xbox协议解析器
      for (uint16_t i = 0; i < packet.len; ++i) {
        uint8_t frame_id = xbox_remote.processByte(packet.data[i]);
        if (frame_id != 0) {
          // 帧解析完成，可以在这里获取控制器数据并做业务处理
          const auto &ctrl_data = xbox_remote.getControllerData();
          (void)ctrl_data; // 使用 ctrl_data 进行你的业务逻辑
          // 示例：发布到消息总线、驱动电机等
        }
      }
    }
    vTaskDelay(1);
  }
}

uint8_t rev_data[64] = {0};
static inline uint16_t processByte(uint8_t byte) {
  static int state = 0;
  static int cnt = 0;
  static int i, flag = 0;
  switch (state) {
  case 0:
    // 帧头1
    if (byte == 0xFE) {
      state = 1;
    }
    break;
  case 1:
    // 帧头2
    if (byte == 0xFF) {
      state = 2;
    } else {
      state = 0;
    }
    break;
  case 2:
    // 字节数
    cnt = byte;
    state = 3;
    break;
  case 3:
    // 搬运
    if (flag == 0) {
      memset(rev_data, 0, 64);
      flag = 1;
      i = 0;
    }
    if (cnt-- > 0) {
      rev_data[i++] = byte;
      break;
    } else {
      state = 4;
      flag = 0;
    }
  case 4:
    // 帧尾1
    if (byte == 0xDC) {
      state = 5;
    } else {
      state = 0;
      memset(rev_data, 0, 64);
    }
    break;
  case 5:
    // 帧尾2
    if (byte == 0xDD) {
      state = 0;
      int ret = i;
      return ret;
    } else {
      state = 0;
      memset(rev_data, 0, 64);
    }
    break;
  }
  return 0;
}

void usbCdcProcessTask(void *argument) {

  (void)argument;

  // 数据实时性要求高，你这个处理任务肯定是优先级要高一点
  for (;;) {
    (void)osSemaphoreAcquire(usbcdc_rx_semphore, osWaitForever);

    UsbPort::Packet packet{};
    while (UsbPort::Instance().Read(packet)) {
      // 逐个字节解析
      for (uint16_t i = 0; i < packet.len; ++i) {
        uint8_t frame_id = ros_protocol.processData(packet.data[i]);
        if (frame_id != 0) {
          uint8_t rev[64] = {0};
          memcpy(rev, ros_protocol.getSensorBagData().i16_data,
                 sizeof(ros_protocol.getSensorBagData().i16_data));
          memcpy(rev + sizeof(ros_protocol.getSensorBagData().i16_data),
                 ros_protocol.getSensorBagData().f_data,
                 sizeof(ros_protocol.getSensorBagData().f_data));
          UsbPort::Instance().WriteAsync(
              rev, sizeof(ros_protocol.getSensorBagData()));
        }
      }
    }
  }
}