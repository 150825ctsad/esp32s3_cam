/* TSL2584.c */
#include "TSL2584.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#define TSL_I2C_ADDR  0x39

float light_DATA = 0;

/* === 1. 一次性常量，放 .rodata，不占栈 === */
static const uint8_t CMD_ENABLE[]   = {0x80 | 0x00, 0x03}; // PON | AEN
static const uint8_t CMD_ATIME[]    = {0x80 | 0x01, 0xD5}; // 101 ms
static const uint8_t CMD_GAIN[]     = {0x80 | 0x0F, 0x02}; // 16x gain
static const uint8_t CMD_ID[]       = {0x80 | 0x12};
static const uint8_t CMD_CH0LOW = 0x80 | 0x14;

/* === 2. 句柄缓存 === */
static i2c_master_dev_handle_t g_i2c = NULL;
i2c_master_dev_handle_t tsl_dev;
/* -------------------------------------------------- */

// TSL2584任务函数
void tsl2584_task(void *arg) {
    (void)arg;
    for (;;) {
        int lux100 = tsl2584_read_lux_x100();   // 整数
        /* 用 %d.%02d 打印，避免链接浮点库 */
        printf("TSL lux: %d.%02d\n", lux100 / 100, lux100 % 100);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void tsl2584_init(i2c_master_dev_handle_t i2c_dev)
{
    g_i2c = i2c_dev;                 // 缓存句柄
    uint8_t id = 0;

    /* 顺序初始化寄存器，失败直接断言 */
    ESP_ERROR_CHECK(i2c_master_transmit(g_i2c, CMD_ENABLE, 2, -1));
    ESP_ERROR_CHECK(i2c_master_transmit_receive(g_i2c, CMD_ID, 1, &id, 1, 20));
    printf("TSL2584 ID: 0x%02X\n", id);

    ESP_ERROR_CHECK(i2c_master_transmit(g_i2c, CMD_ATIME, 2, -1));
    ESP_ERROR_CHECK(i2c_master_transmit(g_i2c, CMD_GAIN,  2, -1));
    xTaskCreatePinnedToCore(tsl2584_task, "TSL2584", 4096, NULL, 5, NULL, 0); // CPU0
}

/* -------------------------------------------------- */
int tsl2584_read_lux_x100(void)
{
    /* 用 16-bit 缓冲，不再截断 */
    uint16_t ch0, ch1;
    uint8_t tx = CMD_CH0LOW;
    uint8_t  rx[4];

    /* 一次读 4 字节：CH0L/H, CH1L/H */
    ESP_ERROR_CHECK(
        i2c_master_transmit_receive(g_i2c, &tx, 1, rx, 4, 20));

    ch0 = (rx[1] << 8) | rx[0];
    ch1 = (rx[3] << 8) | rx[2];

    /* lux = ((ch0 - ch1) * 408) / (ATIME * GAIN)
       ATIME = 101 ms ≈ 100, GAIN = 16 → 分母 1600
       放大 100 倍后返回整数，无浮点 */
    int lux100 = ((int32_t)ch0 - ch1) * 40800L / 1600;
    light_DATA = lux100;
    return lux100 < 0 ? 0 : lux100;
}

