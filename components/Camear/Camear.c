#include <stdio.h>
#include <stdlib.h>
#include "Camear.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_psram.h"
#include "lcd_set.h"
#include "spi_set.h"
//WROVER-KIT PIN Map
#define CAM_PIN_PWDN    -1 //power down is not used
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5

#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11
#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13


#define PSRAM_START_ADDR 0x3F800000  // PSRAM 起始地址
#define PSRAM_SIZE (8 * 240 * 240) // PSRAM 大小（假设为 8MB）

camera_fb_t *picture_data=NULL;

camera_config_t esp32cam_config = {
    .pin_pwdn = -1,
    .pin_reset = -1,

    .pin_xclk = 15,

    .pin_sscb_sda = 4,
    .pin_sscb_scl = 5,

    .pin_d7 = 16,
    .pin_d6 = 17,
    .pin_d5 = 18,
    .pin_d4 = 12,
    .pin_d3 = 10,
    .pin_d2 = 8,
    .pin_d1 = 9,
    .pin_d0 = 11,
    .pin_vsync = 6,
    .pin_href = 7,
    .pin_pclk = 13,
    .xclk_freq_hz = 40000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_RGB565,//PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_240X240,
    .jpeg_quality = 12, // 0-63，数值越小质量越高
    .fb_count = 2       // 如果大于1，i2s将以连续模式运行。仅与jpeg一起使用
};


uint16_t Camearinit(void){

    // 如果没有PSRAM，限制帧大小并使用DRAM
    //    esp32cam_config.frame_size = FRAMESIZE_SVGA;
    esp32cam_config.grab_mode = CAMERA_GRAB_LATEST;
    esp32cam_config.fb_location = CAMERA_FB_IN_PSRAM;
    // 初始化摄像头
    esp_err_t err = esp_camera_init(&esp32cam_config);
    // 检查摄像头是否初始化成功
    if (err != ESP_OK) 
    {
        // 如果初始化失败，打印错误代码
        printf("Camera init failed with error 0x%x", err);
        return 0; // 返回0表示失败
    }
 
  // 获取传感器指针
  sensor_t * s = esp_camera_sensor_get();

    s->set_brightness(s, 1);     // 亮度（-2 到 2）
    s->set_contrast(s, 0);       // 对比度（-2 到 2）
    s->set_saturation(s, -1);     // 饱和度（-2 到 2）
    s->set_sharpness(s, 0);      // 锐度（-2 到 2）
    s->set_aec_value(s, 800);    // 曝光值（0-1200）
    s->set_raw_gma(s, 1);        // RAW GMA（0 关闭，1 开启）
    s->set_lenc(s, 1);           // 镜头校正（0 关闭，1 开启）
    s->set_hmirror(s, 1);        // 水平镜像（0 关闭，1 开启）
    s->set_vflip(s, 1);          // 垂直翻转（0 关闭，1 开启）

  
  printf("Camera configuration complete!");
  return 1; // 返回1表示成功
}

void Camera_app(void)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) { printf("fb NULL\n"); return; }
    if (fb->format != PIXFORMAT_RGB565) 
    {
        printf("Unexpected format: %d\n", fb->format);
        esp_camera_fb_return(fb);
        return;
    }

    draw_area_t draw_area = 
    {
        .x_start = 0,  .y_start = 0,
        .x_end   = 239,.y_end   = 239,   // 坐标修正
        .data    = (uint16_t*)fb->buf
    };
    picture_data = fb;
    // 画图之前：不要任何 printf
    draw(&draw_area);

    esp_camera_fb_return(fb);
}
