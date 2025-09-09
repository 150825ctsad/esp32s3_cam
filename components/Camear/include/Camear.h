#ifndef CAMERA_H
#define CAMERA_H

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_camera.h"

    uint16_t Camearinit(void);
    void Camera_app(void);
    extern camera_fb_t * picture_data;
#endif
