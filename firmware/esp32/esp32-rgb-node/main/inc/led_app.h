#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 GPIO 48 上的板载 WS2812 LED
 */
esp_err_t led_app_init(void);

/**
 * @brief 设置 LED 的 RGB 颜色值
 * @param red 0-255
 * @param green 0-255
 * @param blue 0-255
 */
esp_err_t led_app_set_color(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief 开关 LED 状态（开：默认亮白光，关：熄灭）
 */
esp_err_t led_app_set_state(int state);

#ifdef __cplusplus
}
#endif