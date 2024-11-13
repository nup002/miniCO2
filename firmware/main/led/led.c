#include "led.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <esp_check.h>
#include "led_strip.h"
#include "sdkconfig.h"


static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void initiate_led(void)
{
    ESP_LOGI(TAG, "Initiating the LED");
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_RETURN_ON_ERROR(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip), TAG, "LED initialization failed");

    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void led_task(void *pvParameters){
    esp_err_t led_init_err = initiate_led();
    if (!led_init_err){
        // Log the error and put MINICO2 into error state and return from the task
        ESP_ERROR_CHECK_WITHOUT_ABORT(led_init_err);
        return;
    }

    while (1) {
        
    }
}