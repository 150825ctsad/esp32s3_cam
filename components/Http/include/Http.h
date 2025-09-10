#ifndef HTTP_SERVER_H
#define HTTP_H

#include "esp_http_server.h"
#include "esp_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化HTTP服务器
 * @return ESP_OK 成功
 *         其他值 失败
 */
esp_err_t http_server_init(void);

/**
 * @brief 停止HTTP服务器
 */
void http_server_stop(void);

/**
 * @brief 启动HTTP视频流服务器
 */
void start_http_stream_server(void);


#ifdef __cplusplus
}
#endif

#endif /* HTTP_SERVER_H */

