#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "driver/gpio.h"
#include <esp_err.h>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_timer.h>
#include "led_strip.h"
#include "sdkconfig.h"
#include "../types.h"
#include "led.h"

#define LED_BRIGHTNESS 0.5  // 0 to 1


static const char *LED_TAG = "led";

static int64_t time0 = 0;

static led_strip_handle_t led;

esp_err_t initiate_led(void)
{
    ESP_LOGI(LED_TAG, "Initializing the LED...");
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags = {.invert_out = 0}
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .mem_block_symbols = 0,
        .flags = {.with_dma = false},
    };
    
    ESP_LOGI(LED_TAG, "1/2 - Creating new led_strip_handle_t object");
    ESP_RETURN_ON_ERROR(led_strip_new_rmt_device(&strip_config, &rmt_config, &led), LED_TAG, "LED initialization failed");

    ESP_LOGI(LED_TAG, "2/2 - Clearing LED");
    ESP_RETURN_ON_ERROR(led_strip_clear(led), LED_TAG, "LED clear failed");

    ESP_LOGI(LED_TAG, "LED initialized succesfully");
    return ESP_OK;
}

void set_led_from_RGBA(struct RGBA clr){
    float a_f = clr.a / 255.0;
    ESP_ERROR_CHECK(led_strip_set_pixel(led, 0, clr.r * a_f, clr.g * a_f, clr.b * a_f));
    ESP_ERROR_CHECK(led_strip_refresh(led));
}

void set_visual_led_state_from_state(enum LED_STATES state, struct LED_VISUAL_STATE* led_visual_state){
    if (state == LOW_CO2){
        led_visual_state->clr.r = 0;
        led_visual_state->clr.g = 255;
        led_visual_state->clr.b = 0;
        led_visual_state->mode = STATIC;
    }else if (state == MEDIUM_CO2){
        led_visual_state->clr.r = 255;
        led_visual_state->clr.g = 127;
        led_visual_state->clr.b = 0;
        led_visual_state->mode = STATIC;
    }else if (state == HIGH_CO2){
        led_visual_state->clr.r = 255;
        led_visual_state->clr.g = 0;
        led_visual_state->clr.b = 0;
        led_visual_state->mode = STATIC;
    }else if (state == BOOTING_L){
        led_visual_state->clr.r = 255;
        led_visual_state->clr.g = 255;
        led_visual_state->clr.b = 255;
        led_visual_state->mode = PULSING;
        led_visual_state->period = 2;
    }else if (state == ERROR_L){
        led_visual_state->clr.r = 255;
        led_visual_state->clr.g = 0;
        led_visual_state->clr.b = 0;
        led_visual_state->mode = PULSING;
        led_visual_state->period = 1;
    }else{
        ESP_LOGW(LED_TAG, "Invalid LED state %u", state);
    }
}

// Updates the LED brightness based on the pulsing period.
void progress_led_pulse(struct LED_VISUAL_STATE state){
    uint64_t ms_since_pulsing_began = (esp_timer_get_time() - time0)/1E3;
    
    int32_t period_ms = state.period * 1000;
    int32_t pulse_progress = ms_since_pulsing_began % period_ms;
    float alpha = abs(pulse_progress - period_ms/2)/(float)period_ms/2;

    struct RGBA new_clr = state.clr;
    new_clr.a = new_clr.a * alpha;
    set_led_from_RGBA(new_clr);
}

void led_task(void *pvParameters)
{
    // Get the queues from the pvParameters pointer
    QueueHandle_t *queues = (QueueHandle_t *)pvParameters;
    QueueHandle_t led_state_queue = queues[0];
    QueueHandle_t errors_queue = queues[1];

    // Create a struct to hold the state of the LED;
    struct RGBA led_clr; 
    led_clr.r = 0;
    led_clr.g = 0;
    led_clr.b = 0;
    led_clr.a = 255*LED_BRIGHTNESS;
    struct LED_VISUAL_STATE led_visual_state;
    led_visual_state.clr = led_clr;
    led_visual_state.mode = STATIC;    
    led_visual_state.period = 1;

    esp_err_t led_init_err = initiate_led();
    if (led_init_err){
        // Log the error, put it on the errors queue, and enter an infinite loop
        ESP_ERROR_CHECK_WITHOUT_ABORT(led_init_err);
        xQueueSendToBack(errors_queue, &led_init_err, (TickType_t) 0);
        while (1){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    
    // If led_state_queue is 0 (meaning it failed at being created), we put the LED state to ERROR 
    // and enter an infinite loop
    if (led_state_queue == 0){
        ESP_LOGE(LED_TAG, "Led state queue is 0, entering infinite loop");
        set_visual_led_state_from_state(ERROR_L, &led_visual_state);
        while (1){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    enum LED_STATES state;
    while (1){
        if (xQueueReceive(led_state_queue, &( state), (TickType_t) 10)){
            ESP_LOGD(LED_TAG, "Received LED state %u", state);
            set_visual_led_state_from_state(state, &led_visual_state);
            ESP_LOGI(LED_TAG, "R: %u, G: %u, B: %u, A: %u", led_visual_state.clr.r, led_visual_state.clr.g, led_visual_state.clr.b, led_visual_state.clr.a);
        }

        
        if (led_visual_state.mode == STATIC) {
            set_led_from_RGBA(led_visual_state.clr);
        } else if (led_visual_state.mode == PULSING) {
            progress_led_pulse(led_visual_state);
        }
    }
}