#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "MAX471.h"
#include "SHT20.h"
#include "Camear.h"
#include "JDQ.h"
#include "spi_set.h"
#include "lcd_set.h"
#include "driver/i2c_master.h"
#include "TSL2584.h"
#include "I2C.h"
#include "WIFI_Set.h"

// 声明全局变量


// ADC任务函数
void adc_task(void *pvParameters) {
    for (;;) {
        int mv = TEST_ADC_GetVoltage_mv();
        printf("ADC: %d mV\n", mv);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// 摄像头任务函数
void camera_task(void *pvParameters) {
    while (1) {
        Camera_app(); // 处理摄像头相关功能
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

// 继电器任务函数
void relay_task(void *pvParameters) {
    while (1) {
        relay_on(); // 打开继电器
        vTaskDelay(pdMS_TO_TICKS(5000)); // 每秒执行一次
        // 如果需要关闭继电器，可以添加 relay_off() 和相应的延迟
    }
}

// LCD显示任务函数
void lcd_display_task(void *pvParameters) {
    // 初始化LCD显示
    spi_init();
    lcd_init();
    uint16_t pixel_format = get_pixel_format();
    printf("pixel_format = %04x\n", pixel_format);
    
    while (1) {
        // 更新LCD显示内容
        // 例如：显示传感器数据、状态信息等
        vTaskDelay(pdMS_TO_TICKS(100)); // 每100ms更新一次显示
    }
}


void app_main(void) {
    // 初始化I2C
    esp_err_t ret = i2c_master_init(&sth_dev, &tsl_dev);
    if (ret != ESP_OK) {
        printf("I2C初始化失败: %s\n", esp_err_to_name(ret));
        return;
    }
    
    // 初始化各个外设
    //TEST_ADC_Init();
    sth20_init(sth_dev); // 初始化SHT20传感器
    tsl2584_init(tsl_dev); // 初始化TSL2584传感器
    //Camearinit(); // 初始化摄像头
    relay_init(); // 初始化继电器
    wifi_init_sta(); // 初始化WiFi
    
    // 创建各个任务
    xTaskCreatePinnedToCore(adc_task, "ADC", 4096, NULL, 5, NULL, 0); // CPU0
    //xTaskCreatePinnedToCore(camera_task, "Camera", 8192, NULL, 5, NULL, 1); // CPU1
    //xTaskCreatePinnedToCore(relay_task, "Relay", 2048, NULL, 5, NULL, 0); // CPU0
    //xTaskCreatePinnedToCore(lcd_display_task, "LCD", 8192, NULL, 5, NULL, 1); // CPU1

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}