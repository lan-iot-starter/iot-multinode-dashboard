#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "mqtt_app.h"
#include "led_app.h"
#include "cJSON.h"

static const char *TAG = "MQTT_EXAMPLE";

/* ================== 用户配置区域（宏定义） ================== */
#define MQTT_BROKER_URI "mqtt://YOUR_BROKER_IP:1883" // 替换为你的局域网IP或域名
#define MQTT_USERNAME "YOUR_USERNAME"                // 替换为你的MQTT用户名
#define MQTT_PASSWORD "YOUR_PASSWORD"                // 替换为你的MQTT密码
#define DEVICE_ID "YOUR_DEVICE_UNIQUE_ID"            // 替换为你的设备唯一ID

#define MQTT_SUB_TOPIC "device/" DEVICE_ID "/command"         // 订阅主题
#define MQTT_PUB_TOPIC "client/" DEVICE_ID "/sensor"          // 发布主题
#define MQTT_HEARTBEAT_TOPIC "client/" DEVICE_ID "/heartbeat" // 心跳包发布主题
/* ========================================================== */

// 全局客户端句柄，用于定时任务中发送心跳
static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static TaskHandle_t s_heartbeat_task_handle = NULL;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

// 心跳包发送任务：每 60 秒发送一次
static void heartbeat_task(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    char payload[128];

    while (1)
    {
        // 获取当前系统时间戳（毫秒级，用 esp_timer 获取开机以来的毫秒数或替换为真实时间）
        int64_t ts = esp_timer_get_time() / 1000;

        // 组装要求的 JSON 格式
        snprintf(payload, sizeof(payload),
                 "{\n  \"type\": \"heartbeat\",\n  \"status\": \"online\",\n  \"sign\": \"%s\",\n  \"ts\": %lld\n}",
                 DEVICE_ID, ts);

        // 发送心跳 QoS 0
        int msg_id = esp_mqtt_client_publish(client, MQTT_HEARTBEAT_TOPIC, payload, 0, 0, 0);
        ESP_LOGI(TAG, "Sent heartbeat, msg_id=%d", msg_id);

        // 每 60 秒发送一次
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}

// 解析发来数据
static void handle_mqtt_command(const char *data, int data_len)
{
    if (data_len <= 0 || data == NULL)
        return;

    char *data_str = malloc(data_len + 1);
    if (data_str == NULL)
    {
        ESP_LOGW(TAG, "Failed to allocate memory for MQTT data");
        return;
    }

    memcpy(data_str, data, data_len);
    data_str[data_len] = '\0';

    cJSON *root = cJSON_Parse(data_str);
    if (root != NULL)
    {
        cJSON *j_r = cJSON_GetObjectItem(root, "r");
        cJSON *j_g = cJSON_GetObjectItem(root, "g");
        cJSON *j_b = cJSON_GetObjectItem(root, "b");

        if (cJSON_IsNumber(j_r) && cJSON_IsNumber(j_g) && cJSON_IsNumber(j_b))
        {
            int r = j_r->valueint;
            int g = j_g->valueint;
            int b = j_b->valueint;

            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);

            led_app_set_color((uint8_t)r, (uint8_t)g, (uint8_t)b);
            ESP_LOGI(TAG, "Command received: Set RGB color R:%d G:%d B:%d",r,g,b);
        }else{
            ESP_LOGW(TAG, "JSON fields 'r','g','b' missing or invalid");
        }
        cJSON_Delete(root);
    }else{
        ESP_LOGW(TAG, "Failed to parse JSON: %s", data_str);
    }
    free(data_str);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        // 发布上线传感器消息
        msg_id = esp_mqtt_client_publish(client, MQTT_PUB_TOPIC, "Hello ESP32", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        // 订阅指令主题
        msg_id = esp_mqtt_client_subscribe(client, MQTT_SUB_TOPIC, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // 连接成功后，启动心跳任务（如果尚未创建）
        if (s_heartbeat_task_handle == NULL)
        {
            xTaskCreate(heartbeat_task, "heartbeat_task", 4096, client, 5, &s_heartbeat_task_handle);
        }
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        // 断开连接时可以视情况挂起或删除心跳任务，这里保持运行或由任务内部处理
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        handle_mqtt_command(event->data, event->data_len);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_mqtt_client);
}