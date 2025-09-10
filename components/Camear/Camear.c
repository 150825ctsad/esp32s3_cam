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
#include "jpeg_decoder.h"
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
    .pixel_format = PIXFORMAT_JPEG,//PIXFORMAT_JPEG,PIXFORMAT_RGB565
    .frame_size = FRAMESIZE_240X240,
    .jpeg_quality = 5, // 0-63，数值越小质量越高
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
    
    s->set_brightness(s, 0);        // 亮度(-2到2)，0为默认值
    
    s->set_contrast(s, 2);          // 对比度(-2到2)，增加对比度使图像更清晰
    s->set_saturation(s, 2);        // 饱和度(-2到2)，增加饱和度使颜色更鲜艳
    s->set_sharpness(s, 2);         // 锐度(-2到2)，增加锐度使细节更清晰
    
    s->set_denoise(s, 1);           // 降噪级别(0-3)，1为轻度降噪，平衡画质和细节
    
    // 曝光控制设置
    s->set_exposure_ctrl(s, 1);     // 启用自动曝光控制
    
    s->set_aec2(s, 1);              // 启用高级曝光控制
    
    s->set_ae_level(s, 1);          // 曝光补偿(-2到2)，0为默认
    
    s->set_aec_value(s, 1200);       // 曝光值(0-1200)，适中值，可根据环境调整
    
    // 增益控制设置
    s->set_gain_ctrl(s, 1);         // 启用自动增益控制
    
    s->set_gainceiling(s, GAINCEILING_2X); // 增益上限，设为2X以减少噪点
    
    // 白平衡设置
    s->set_whitebal(s, 1);          // 启用自动白平衡
    
    s->set_awb_gain(s, 1);          // 启用自动白平衡增益
    
    s->set_wb_mode(s, 0);           // 白平衡模式(0-4)，0为自动模式
    
    // 图像增强和校正
    s->set_dcw(s, 1);               // 启用动态对比度增强
    
    s->set_bpc(s, 1);               // 启用坏像素校正
    
    s->set_wpc(s, 1);               // 启用白平衡校正
    
    s->set_raw_gma(s, 1);           // 启用RAW数据伽马校正
    
    s->set_lenc(s, 1);              // 启用镜头畸变校正
    
    // 图像方向设置(根据实际安装方向调整)
    s->set_hmirror(s, 1);           // 水平镜像(0关闭，1开启)
    s->set_vflip(s, 0);             // 垂直翻转(0关闭，1开启)

  
  printf("Camera configuration complete!");
  return 1; // 返回1表示成功
}

void Camera_app(void)
{
    //获取摄像头帧缓冲
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) { 
        printf("fb NULL\n"); 
        return; 
    }
    
    //检查是否为 JPEG 格式
    if (fb->format != PIXFORMAT_JPEG) 
    {
        printf("Unexpected format: %d\n", fb->format);
        esp_camera_fb_return(fb);
        return;
    }
    
    picture_data = fb;  // 保存图像数据指针

    //定义 JPEG 解码配置
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = fb->buf,                     // 输入 JPEG 数据
        .indata_size = fb->len,                // 输入数据大小
        .out_format = JPEG_IMAGE_FORMAT_RGB565, // 输出格式为 RGB565
        .out_scale = JPEG_IMAGE_SCALE_0,       // 缩放
        .flags.swap_color_bytes = 1,           // 不交换颜色字节
        .advanced.working_buffer = NULL,       // 让库自动分配工作缓冲区
        .advanced.working_buffer_size = 0      // 自动计算工作缓冲区大小
    };

    //获取图像信息，用于分配输出缓冲区
    esp_jpeg_image_output_t img_info;
    if (esp_jpeg_get_image_info(&jpeg_cfg, &img_info) != ESP_OK) {
        printf("Failed to get JPEG image info\n");
        esp_camera_fb_return(fb);
        return;
    }

    //分配输出缓冲区
    uint8_t *out_buf = (uint8_t *)heap_caps_malloc(img_info.output_len, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!out_buf) {
        printf("Failed to allocate output buffer\n");
        esp_camera_fb_return(fb);
        return;
    }
    
    //设置输出缓冲区
    jpeg_cfg.outbuf = out_buf;
    jpeg_cfg.outbuf_size = img_info.output_len;

    //解码 JPEG 图像
    if (esp_jpeg_decode(&jpeg_cfg, &img_info) != ESP_OK) {
        printf("Failed to decode JPEG image\n");
        free(out_buf);
        esp_camera_fb_return(fb);
        return;
    }

    //准备绘制区域
    draw_area_t draw_area = {
        .x_start = 0,  
        .y_start = 0,
        .x_end   = 239, 
        .y_end   = 239,   // 坐标修正为 240x240 LCD 尺寸
        .data    = (uint16_t*)out_buf  // 解码后的 RGB565 数据
    };
    
    //显示图像
    draw(&draw_area);

    //清理资源
    free(out_buf);
    esp_camera_fb_return(fb);
}