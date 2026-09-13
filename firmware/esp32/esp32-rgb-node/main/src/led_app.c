#include "led_app.h"
#include "led_strip.h"
#include "esp_log.h"

#define LED_GPIO_PIN 48
#define LED_STRIP_NUM 1

static const char *TAG = "LED_APP";
static led_strip_handle_t s_led_strip = NULL;

esp_err_t led_app_init(void)
{
    ESP_LOGI(TAG, "Initializing WS2812 on GPIO %d...", LED_GPIO_PIN);

    // 1. 配置 LED 灯条基础参数
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO_PIN,
        .max_leds = LED_STRIP_NUM,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // 修改此处
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    // 2. 配置 RMT 后端参数
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };

    // 3. 创建 led_strip 设备句柄
    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create led_strip device: %s", esp_err_to_name(err));
        return err;
    }

    // 初始化完成后默认清除/熄灭 LED
    led_strip_clear(s_led_strip);
    ESP_LOGI(TAG, "WS2812 initialized successfully.");
    return ESP_OK;
}

esp_err_t led_app_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    if (s_led_strip == NULL) {
        ESP_LOGE(TAG, "LED strip not initialized!");
        return ESP_ERR_INVALID_STATE;
    }

    // 设置第一个灯珠（索引 0）的 RGB 像素值
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, red, green, blue));
    // 刷新灯条使颜色生效
    return led_strip_refresh(s_led_strip);
}

esp_err_t led_app_set_state(int state)
{
    if (state) {
        // 开启：亮微光（避免过亮刺眼）
        return led_app_set_color(20, 20, 20);
    } else {
        // 关闭：清空像素
        if (s_led_strip == NULL) return ESP_ERR_INVALID_STATE;
        return led_strip_clear(s_led_strip);
    }
}