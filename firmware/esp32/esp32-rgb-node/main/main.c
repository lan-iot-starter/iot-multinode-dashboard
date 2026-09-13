#include <stdio.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "wifi_app.h" // 引入你的 Wi-Fi 头文件
#include "mqtt_app.h" // 引入你的 MQTT 头文件
#include "led_app.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");

    // 初始化系统基础组件
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    led_app_init();
    // 1. 初始化并连接 Wi-Fi（使用 wifi_app.c 里的宏定义账号密码）
    wifi_init_sta();

    // 2. 启动 MQTT 客户端（使用 mqtt_app.c 里的宏定义配置）
    mqtt_app_start();
}