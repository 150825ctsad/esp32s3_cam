#ifndef _LCD_SET_H_
#define _LCD_SET_H_
#include "spi_set.h"
#ifndef MIN
#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#endif

typedef struct {
    uint16_t x_start;
    uint16_t y_start;
    uint16_t x_end;
    uint16_t y_end;
    uint16_t *data;
} draw_area_t;

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len);
void lcd_cmd(spi_device_handle_t handle, uint8_t cmd, bool keep_cs_active);
void lcd_init();
uint16_t get_pixel_format();
esp_err_t draw(draw_area_t *area);
void show_test_picture();
#endif
