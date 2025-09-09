#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "UART_Set.h"
#include "freertos/FreeRTOS.h"
#define UART_NUM UART_NUM_0  // 使用UART_NUM_0作为默认UART端口

static uint8_t uart_buffer[8];

void Uart_Task(void *pvParameters) {
    while(1) {
        int len = uart_read_bytes(UART_NUM, uart_buffer, sizeof(uart_buffer)-1, pdMS_TO_TICKS(100));
        if(len > 0) {
            // 安全终止缓冲区
            uart_buffer[len] = '\0';
            
            // 回显数据
            uart_write_bytes(UART_NUM, (const char*)uart_buffer, len);
        }
        taskYIELD();
    }
}


void user_uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    // Install UART driver
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, GPIO_NUM_21, GPIO_NUM_20, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, 2048, 0, 0, NULL, 0));
    ESP_LOGI("UART", "UART initialized successfully");

    xTaskCreate(Uart_Task, "Uart_Task", 2048, NULL, 10, NULL);
}
