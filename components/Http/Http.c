#include <esp_log.h>
#include <esp_system.h>
#include <esp_http_server.h>
#include <esp_timer.h>
#include "esp_camera.h"
#include "Http.h"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n";

static const char* TAG = "HTTP_SERVER";
static httpd_handle_t server = NULL;

/**
 * @brief HTTP服务器处理函数：提供MJPEG视频流
 */
static esp_err_t jpg_stream_handler(httpd_req_t *req)
{
    extern camera_fb_t *picture_data;
    esp_err_t res = ESP_OK;
    if (!picture_data) {
        ESP_LOGE(TAG, "No camera frame available");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    if (picture_data->format != PIXFORMAT_JPEG) {
        ESP_LOGE(TAG, "Frame is not JPEG format");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    res = httpd_resp_set_type(req, "image/jpeg");
    if (res != ESP_OK) {
        return res;
    }
    res = httpd_resp_send(req, (const char *)picture_data->buf, picture_data->len);
    return res;
}

/**
 * @brief HTTP服务器处理函数：提供简单的HTML页面
 */
static esp_err_t index_handler(httpd_req_t *req)
{
    const char* index_html = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>ESP32-CAM Stream</title>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>ESP32-CAM Video Stream</h1>\n"
        "    <img src='/stream' />\n"
        "</body>\n"
        "</html>\n";
    
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, index_html, strlen(index_html));
}

/**
 * @brief 初始化HTTP服务器
 */
esp_err_t http_server_init(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 20;

    // 启动HTTP服务器
    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // 注册URI处理程序
        httpd_uri_t stream_uri = {
            .uri       = "/stream",
            .method    = HTTP_GET,
            .handler   = jpg_stream_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &stream_uri);
        
        httpd_uri_t index_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &index_uri);
        
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Failed to start HTTP server");
    return ESP_FAIL;
}

/**
 * @brief 停止HTTP服务器
 */
void http_server_stop(void)
{
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}

/**
 * @brief 启动HTTP视频流服务器（供WIFI_Set调用）
 */
void start_http_stream_server(void)
{
    http_server_init();
}