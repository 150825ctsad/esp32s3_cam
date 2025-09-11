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

static esp_err_t jpg_stream_handler(httpd_req_t *req)
{
    extern camera_fb_t *picture_data;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char part_buf[64];

    static int64_t last_frame = 0;
    if (!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    while (true) {
        if (!picture_data) {
            ESP_LOGE(TAG, "No camera frame available");
            continue;
        }
        if (picture_data->format != PIXFORMAT_JPEG) {
            ESP_LOGE(TAG, "Frame is not JPEG format");
            continue;
        }
        _jpg_buf_len = picture_data->len;
        _jpg_buf = picture_data->buf;
        
    res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    if (res != ESP_OK) break;
    size_t hlen = snprintf(part_buf, sizeof(part_buf), _STREAM_PART, _jpg_buf_len);
    res = httpd_resp_send_chunk(req, part_buf, hlen);
    if (res != ESP_OK) break;
    res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    if (res != ESP_OK) break;
  
    }

    last_frame = 0;
    return res;
}


static esp_err_t index_handler(httpd_req_t *req)
{
    const char* index_html = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>ESP32-CAM Stream</title>\n"
        "    <script>\n"
        "    function refreshImg() {\n"
        "        var img = document.getElementById('cam');\n"
        "        img.src = '/stream?' + new Date().getTime();\n"
        "    }\n"
        "    setInterval(refreshImg, 200);\n"
        "    </script>\n"
        "</head>\n"
        "<body>\n"
        "    <img id='cam' src='/stream' />\n"
        "</body>\n"
        "</html>\n";
    
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, index_html, strlen(index_html));
}


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


void http_server_stop(void)
{
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}


void start_http_stream_server(void)
{
    http_server_init();
}