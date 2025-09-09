#include "spi_set.h"

spi_device_handle_t dev_handle;

void spi_pre_cb(spi_transaction_t *trans)
{
    int dc = (int)trans->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void spi_post_cb(spi_transaction_t *trans)
{
    // 留空或仅用于调试！！！！！
    // 背光应该保持常亮，不随SPI传输变化
}

void spi_init()
{
    //esp32 idf 初始化spi
    spi_bus_config_t bus_config = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .flags = 0,
        .max_transfer_sz = 0,//不限制
    };
    spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);

    //printf("SPI bus initialized\n");

    spi_device_interface_config_t dev_config = {
        .clock_source = SPI_CLK_SRC_DEFAULT,
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = 40*1000*1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .pre_cb = spi_pre_cb,
        .post_cb = spi_post_cb,
        .queue_size = 7,  
    };
    spi_bus_add_device(SPI2_HOST, &dev_config, &dev_handle);

    //printf("SPI device added\n");
}
