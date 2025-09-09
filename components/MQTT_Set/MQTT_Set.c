/* MQTT_Set.c  精简版：纯任务，无软件定时器 */
#include <stdio.h>
#include "MQTT_Set.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SHT20.h"
#include "Camear.h"
#include "JDQ.h"
#define TAG                    "MQTT_CLIENT"
#define MQTT_BROKER_URI        "mqtt://192.168.248.70"
#define MQTT_BROKER_PORT       1883
#define MQTT_CLIENT_ID         "client_id1515"
#define MQTT_USERNAME          "my_username21"
#define MQTT_PASSWORD          "my_password"
#define MQTT_TOPIC_SENSOR      "test/ESP-IDF/SENSOR_DATA"
#define MQTT_TOPIC_COMMAND     "test/ESP-IDF/COMMAND"
#define MQTT_TOPIC_CAM         "test/ESP-IDF/CAM"
static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;
static int reconnect_attempts = 0;

/* ---------- MQTT 事件回调 ---------- */
static void mqtt_event_callback(void *handler_args,
                                esp_event_base_t base,
                                int32_t event_id,
                                void *event_data)
{
    esp_mqtt_event_handle_t data = (esp_mqtt_event_handle_t)event_data;

    switch (event_id) {
    case MQTT_EVENT_CONNECTED:
        esp_mqtt_client_subscribe_single(mqtt_client, MQTT_TOPIC_SENSOR, 0);
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        mqtt_connected = true;
        reconnect_attempts = 0; // 重置重连尝试计数
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqtt_connected = false;
        break;

    case MQTT_EVENT_ERROR:
        if (data->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "Transport error: errno=%d", data->error_handle->esp_transport_sock_errno);
        } else if (data->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGE(TAG, "Connection refused: %d", data->error_handle->connect_return_code);
            
            // 根据返回码处理不同的拒绝原因
            switch (data->error_handle->connect_return_code) {
                case 1:
                    ESP_LOGE(TAG, "Unacceptable protocol version");
                    break;
                case 2:
                    ESP_LOGE(TAG, "Identifier rejected");
                    break;
                case 3:
                    ESP_LOGE(TAG, "Server unavailable");
                    break;
                case 4:
                    ESP_LOGE(TAG, "Bad username or password");
                    break;
                case 5:
                    ESP_LOGE(TAG, "Not authorized");
                    break;
                default:
                    ESP_LOGE(TAG, "Unknown refusal reason");
                    break;
            }
        }
        break;

    case MQTT_EVENT_DATA:
        //ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        //printf("topic: %.*s\r\n", data->topic_len, data->topic);
        //printf("data:  %.*s\r\n", data->data_len, data->data);
        if (data->data_len == 2 && strncmp((const char *)data->data, "ON", 2) == 0) {
            relay_on();
        } else if (data->data_len == 3 && strncmp((const char *)data->data, "OFF", 3) == 0) {
            relay_off();
        }
        break;

    default:
        break;
    }
}

/* ---------- 启动 MQTT ---------- */
static void mqtt_start(void)
{
    // 如果已有客户端实例，先销毁它
    if (mqtt_client != NULL) {
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        vTaskDelay(pdMS_TO_TICKS(1000)); // 等待资源释放
    }

    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
            .address.port = MQTT_BROKER_PORT,
        },
        .credentials = {
            .client_id = MQTT_CLIENT_ID,
            .username = MQTT_USERNAME,
            .authentication.password = MQTT_PASSWORD,
        },
        .session = {
            .keepalive = 60, // 减少心跳间隔
            .disable_clean_session = false,
        },
        .network = {
            .timeout_ms = 10000,
            .reconnect_timeout_ms = 5000,
            .disable_auto_reconnect = false, // 启用自动重连
        },
        .task = {
            .priority = 5,
            .stack_size = 8192, // 增加栈大小
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_callback, NULL);
    esp_err_t err = esp_mqtt_client_start(mqtt_client);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "MQTT start failed: %s", esp_err_to_name(err));
    }
}

/* ---------- 发布传感器数据 ---------- */
static void publish_sensor_data(void)
{
    if (!mqtt_connected) {
        ESP_LOGW(TAG, "Not connected, skipping publish");
        return;
    }

    /* 1. 把全局原始值换算成浮点 */
    double temp = ((double)STH20_WData / 65536.0) * 175.72 - 46.85;
    double humi = ((double)STH20_SData / 65536.0) * 125.0 - 6.0;

    /* 2. 构造 JSON（这里用整数/浮点均可） */
    static char json[256];
    int len = snprintf(json, sizeof(json),
        "{\"timestamp\":%lu,\"temp\":%.2f,\"humi\":%.2f,"
        "\"light\":%.1f,\"adc\":%d,\"device\":\"%s\",\"fw\":\"1.0.0\"}",
        (unsigned long)(xTaskGetTickCount() * portTICK_PERIOD_MS),
        temp, humi, 450.8f, 1024, MQTT_CLIENT_ID);

    if (len > 0 && len < sizeof(json)) {
        int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_SENSOR, json, 0, 0, 0);
        if (msg_id < 0) 
        {
            ESP_LOGE(TAG, "Publish failed: %d", msg_id);
        } 
        else 
        {
            //ESP_LOGI(TAG, "Published: %s", json);
        }

        int msg_cam = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_CAM, (const char *)picture_data->buf, picture_data->len, 0, 0);
        //ESP_LOGI(TAG, "Published: %s", (const char *)picture_data);
    }
}

/* ---------- 主任务 ---------- */
void mqtt_task(void *pvParameters)
{
    // 等待网络连接
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    mqtt_start();

    while (1) {
        /* 未连接就重连 */
        if (!mqtt_connected) {
            reconnect_attempts++;
            ESP_LOGW(TAG, "MQTT reconnecting... (attempt %d)", reconnect_attempts);
            
            // 指数退避策略
            int delay_ms = 1000 * (1 << (reconnect_attempts > 6 ? 6 : reconnect_attempts));
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
            
            mqtt_start();
            continue;
        }

        /* 每 0.1 秒发布一次数据 */
        publish_sensor_data();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}