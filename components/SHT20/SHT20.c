#include <stdio.h>
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// STH20传感器参数
#define STH20_ADDR         0x40       // STH20 7位I2C地址(0x40，对应原代码的0x80写地址)
#define WENDU_COMMAND      0xF3       // 温度测量命令
#define SHIDU_COMMAND      0xF5       // 湿度测量命令
#define STHDU_WHO_I        0xE7       // 设备ID查询命令
#define STHDU_FU_WEI       0XFE
// 全局变量存储传感器数据
uint16_t STH20_WData = 0;  // 温度原始数据
uint16_t STH20_SData = 0;  // 湿度原始数据
i2c_master_dev_handle_t sth_dev;

void Z_STH20_GetData(i2c_master_dev_handle_t i2c_device_handle,uint8_t command) {
    uint8_t data[3];  // 存储读取到的3个字节(高8位+低8位+CRC)
    esp_err_t ret;

    // 1. 发送测量命令
    ret = i2c_master_transmit(i2c_device_handle, &command, 1, -1);
    if (ret != ESP_OK) {
        printf("发送命令失败: %d\n", ret);
        return;
    }

    // 2. 等待测量完成(STH20最大测量时间约85ms)
    vTaskDelay(pdMS_TO_TICKS(90));

    // 3. 读取测量结果(最多重试10次)
    int count = 0;
    do {
        ret = i2c_master_receive(i2c_device_handle, data, 3, -1);
        if (ret == ESP_OK) break;
        vTaskDelay(pdMS_TO_TICKS(10));
    } while (++count < 10);

    if (count >= 10) {
        printf("读取数据超时\n");
        return;
    }

    // 4. 处理原始数据(屏蔽最后2位状态位)
    uint16_t raw_data = (data[0] << 8) | data[1];
    raw_data &= 0xFFFC;

    // 5. 根据命令类型存储数据
    if (command == WENDU_COMMAND) {
        STH20_WData = raw_data;
    } else {
        STH20_SData = raw_data;
    }
}

void sth20_app(i2c_master_dev_handle_t i2c_device_handle) {
        // 读取温度和湿度
        Z_STH20_GetData(i2c_device_handle, WENDU_COMMAND);
        Z_STH20_GetData(i2c_device_handle, SHIDU_COMMAND);

        // // 计算温度(公式来自STH20数据手册)
        // double wendu = STH20_WData;
        // wendu = (wendu / 65536.0) * 175.72 - 46.85;
        // // 计算湿度(公式来自STH20数据手册)
        // double shidu = STH20_SData;
        // shidu = (shidu / 65536.0) * 125 - 6;

        // 打印结果(原代码中的OLED显示替换为串口打印)
        // printf("温度: %.2f°C  |  原始温度值: %d\n", wendu, STH20_WData);
        // printf("湿度: %.2f%%   |  原始湿度值: %d\n", shidu, STH20_SData);
        // printf("-------------------------\n");

        // 每500ms读取一次
        vTaskDelay(pdMS_TO_TICKS(1000));

}

// SHT20任务函数
void sht20_task(void *pvParameters) {
    while (1) {
        sth20_app(sth_dev); // 获取SHT20传感器数据
        vTaskDelay(pdMS_TO_TICKS(5000)); // 每5秒获取一次SHT20数据
    }
}
void sth20_init(i2c_master_dev_handle_t i2c_device_handle) {
   STH20_SData = 0;
   STH20_WData = 0;
   uint8_t command = STHDU_WHO_I,command1 = STHDU_FU_WEI,reg;
   i2c_master_transmit(i2c_device_handle,&command1, 1, -1);
   vTaskDelay(pdMS_TO_TICKS(20));
   i2c_master_transmit_receive(i2c_device_handle, &command, 1, &reg, 1, -1);
   i2c_master_receive(i2c_device_handle, &reg, 1, -1);
   xTaskCreatePinnedToCore(sht20_task, "SHT20", 4096, NULL, 5, NULL, 0); // CPU0
}

    