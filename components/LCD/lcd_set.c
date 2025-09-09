#include "lcd_set.h"
#include "picture.h"

extern spi_device_handle_t dev_handle;

DRAM_ATTR static const lcd_init_cmd_t lcd_init_cmds[] = {
    /* Power contorl B, power control = 0, DC_ENA = 1 */
    {0xCF, {0x00, 0x83, 0X30}, 3},
    /* Power on sequence control,
     * cp1 keeps 1 frame, 1st frame enable
     * vcl = 0, ddvdh=3, vgh=1, vgl=2
     * DDVDH_ENH=1
     */
    {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
    /* Driver timing control A,
     * non-overlap=default +1
     * EQ=default - 1, CR=default
     * pre-charge=default - 1
     */
    {0xE8, {0x85, 0x01, 0x79}, 3},
    /* Power control A, Vcore=1.6V, DDVDH=5.6V */
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    /* Pump ratio control, DDVDH=2xVCl */
    {0xF7, {0x20}, 1},
    /* Driver timing control, all=0 unit */
    {0xEA, {0x00, 0x00}, 2},
    /* Power control 1, GVDD=4.75V */
    {0xC0, {0x26}, 1},
    /* Power control 2, DDVDH=VCl*2, VGH=VCl*7, VGL=-VCl*3 */
    {0xC1, {0x11}, 1},
    /* VCOM control 1, VCOMH=4.025V, VCOML=-0.950V */
    {0xC5, {0x35, 0x3E}, 2},
    /* VCOM control 2, VCOMH=VMH-2, VCOML=VML-2 */
    {0xC7, {0xBE}, 1},
    /* Memory access contorl, MX=MY=0, MV=1, ML=0, BGR=1, MH=0 */
    {0x36, {0x48}, 1},
    /* Pixel format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x55}, 1},
    /* Frame rate control, f=fosc, 70Hz fps */
    {0xB1, {0x00, 0x1B}, 2},
    /* Enable 3G, disabled */
    {0xF2, {0x08}, 1},
    /* Gamma set, curve 1 */
    {0x26, {0x01}, 1},
    /* Positive gamma correction */
    {0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
    /* Negative gamma correction */
    {0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
    /* Column address set, SC=0, EC=0xEF */
    {0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
    /* Page address set, SP=0, EP=0x013F */
    {0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
    /* Memory write */
    {0x2C, {0}, 0},
    /* Entry mode set, Low vol detect disabled, normal display */
    {0xB7, {0x07}, 1},
    /* Display function control */
    {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
    /* Sleep out */
    {0x11, {0}, 0x80},
    /* Display on */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff},
};

void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0) {
        return;    //no need to send anything
    }
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = len * 8;             //Len is in bytes, transaction length is in bits.
    t.tx_buffer = data;             //Data
    t.user = (void*)1;              //D/C needs to be set to 1
    ret = spi_device_polling_transmit(spi, &t); //Transmit!
    assert(ret == ESP_OK);          //Should have had no issues.
}

//发送命令
void lcd_cmd(spi_device_handle_t handle, uint8_t cmd, bool keep_cs_active)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &cmd;
    t.rx_buffer = NULL;
    t.user = (void*)0;
    if (keep_cs_active) {
        t.flags = SPI_TRANS_CS_KEEP_ACTIVE;   //Keep CS active after data transfer
    }
    spi_device_polling_transmit(handle, &t);
}

void lcd_init()
{   
    //初始化PIN_NUM_RST，PIN_NUM_RST，PIN_NUM_DC管脚为推挽输出
    gpio_config_t pGPIOConfig = {
        .pin_bit_mask = (1ULL << PIN_NUM_RST) | (1ULL << PIN_NUM_DC) | (1ULL << PIN_BK_LIGHT),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&pGPIOConfig);

    gpio_set_level(PIN_NUM_RST,0);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_BK_LIGHT,1);
    //vTaskDelay(1000 / portTICK_PERIOD_MS);

    //printf("LCD GPIO initialized\n");
    int cmd = 0;
    while (lcd_init_cmds[cmd].databytes != 0xff) {
        lcd_cmd(dev_handle, lcd_init_cmds[cmd].cmd, false);
        lcd_data(dev_handle, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes & 0x1F);
        if (lcd_init_cmds[cmd].databytes & 0x80) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        cmd++;
    }

    //printf("LCD initialized\n");

}

uint16_t get_pixel_format()
{
    // When using SPI_TRANS_CS_KEEP_ACTIVE, bus must be locked/acquired
    spi_device_acquire_bus(dev_handle, portMAX_DELAY);

    lcd_cmd(dev_handle, 0x0C, true);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8*2;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void*)1;

    spi_device_polling_transmit(dev_handle, &t);

    // Release bus
    spi_device_release_bus(dev_handle);
    return *((uint16_t*)t.rx_data);
}

esp_err_t draw(draw_area_t *area)
{
    // 计算区域宽度和高度
    uint16_t width = area->x_end - area->x_start + 1;
    uint16_t height = area->y_end - area->y_start + 1;
    uint32_t total  = width * height * sizeof(uint16_t);   // 总字节数
    uint16_t *buf   = area->data;                          // 像素首地址

    spi_transaction_t trans_set_area[4] = {
        // trans[0]: 发送列地址设置命令 (0x2A)
        {
            .length = 8,                // 传输 8 位 (1字节)
            .user = (void*)0,           // DC=0 (命令模式)
            .flags = SPI_TRANS_USE_TXDATA, // 使用内部 tx_data 缓冲区
            .tx_data = {0x2A}           // 列地址设置命令
        },

        // trans[1]: 发送列地址范围数据
        {
            .length = 32,               // 传输 32 位 (4字节)
            .user = (void*)1,           // DC=1 (数据模式)
            .flags = SPI_TRANS_USE_TXDATA, // 使用内部 tx_data 缓冲区
            .tx_data = {
                area->x_start >> 8,                 // 起始列高字节 (0x00)
                area->x_start & 0xff,                   // 起始列低字节 (0x00)
                area->x_end >> 8,   // 结束列高字节 (例如: 239>>8=0x00)
                area->x_end & 0xff  // 结束列低字节 (例如: 239&0xFF=0xEF)
            }
        },
        
        // trans[2]: 发送行地址设置命令 (0x2B)
        {
            .length = 8,                // 传输 8 位 (1字节)
            .user = (void*)0,           // DC=0 (命令模式)
            .flags = SPI_TRANS_USE_TXDATA, // 使用内部 tx_data 缓冲区
            .tx_data = {0x2B}           // 行地址设置命令
        },
        
        // trans[3]: 发送行地址范围数据
        {
            .length = 32,               // 传输 32 位 (4字节)
            .user = (void*)1,           // DC=1 (数据模式)
            .flags = SPI_TRANS_USE_TXDATA, // 使用内部 tx_data 缓冲区
            .tx_data = {
                area->y_start >> 8,                   // 起始行高字节 (0x00)
                area->y_start & 0xff,                   // 起始行低字节 (0x00)
                area->y_end >> 8,  // 结束行高字节 (例如: 319>>8=0x01)
                area->y_end & 0xff // 结束行低字节 (例如: 319&0xFF=0x3F)
            }
        },
    };
    //Queue all transactions.
    for (int x = 0; x < 4; x++) {
        spi_device_queue_trans(dev_handle, &trans_set_area[x], portMAX_DELAY);
    }

    
    spi_transaction_t cmd = {
        .length = 8,
        .user   = (void*)0,
        .flags  = SPI_TRANS_USE_TXDATA,
        .tx_data = {0x2C}
    };
    spi_device_polling_transmit(dev_handle, &cmd);
    for (size_t offset = 0; offset < total; offset += CHUNK_BYTES) {
            size_t len = MIN(CHUNK_BYTES, total - offset);

            spi_transaction_t t = {
                .tx_buffer = (uint8_t *)buf + offset, // 当前块首地址
                .length    = len * 8,                 // bit 长度
                .user      = (void*)1,
                .flags     = 0
            };
            spi_device_polling_transmit(dev_handle, &t);   // 阻塞发完这一块
            //printf("Sending chunk: %zu bytes\n", len);
        }

        return ESP_OK;
}

void show_test_picture()
{
    static draw_area_t area = {
        .x_start = 0,
        .y_start = 0,
        .x_end = 161,
        .y_end = 161,
        .data = (uint16_t *)picture,
    };
    draw(&area);
}