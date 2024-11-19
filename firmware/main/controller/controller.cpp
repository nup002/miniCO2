// #ifdef __cplusplus
// extern "C" {
// #endif
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "driver/gpio.h"
#include <esp_err.h>
#include <esp_check.h>
#include <esp_log.h>
#include "sdkconfig.h"
#include "../types.h"

static const char *TAG = "MINICO2";
static enum DEVICE_STATES DEVICE_STATE = BOOTING; 

void set_led_state_from_co2(uint16_t co2, QueueHandle_t led_state_queue){
    enum LED_STATES state;
    if (co2 < 1000){
        state = LOW_CO2;
    }else if (co2 < 2000){
        state = MEDIUM_CO2;
    }else{
        state = HIGH_CO2;
    }
    xQueueSendToBack(led_state_queue, &state, (TickType_t)0);
}

void handle_measurement(struct SCD40measurement meas, QueueHandle_t led_state_queue){
    // Log the measurement to the serial connection
    ESP_LOGI(TAG, "CO2: %u ppm", meas.co2);
    ESP_LOGI(TAG, "Temperature: %.2f °C", meas.temperature);
    ESP_LOGI(TAG, "Humidity: %.2f %%", meas.humidity);

    // Set the LED color based on the CO2 level
    set_led_state_from_co2(meas.co2, led_state_queue);
}


esp_err_t set_device_state(enum DEVICE_STATES state, QueueHandle_t led_state_queue){
    enum DEVICE_STATES old_state = DEVICE_STATE;
    DEVICE_STATE = state;
    LED_STATES led_state;
    ESP_LOGD(TAG, "Device state changing from %s to %s", xstr(old_state), xstr(DEVICE_STATE));
    switch (DEVICE_STATE)
    {
    case BOOTING:
        led_state = BOOTING_L;
        xQueueSendToBack(led_state_queue, &led_state, (TickType_t)0);
        break;
    case ERROR:
        led_state = ERROR_L;
        xQueueSendToBack(led_state_queue, &led_state, (TickType_t)0);
        while (1){
            ESP_LOGE(TAG, "MINICO2 is in ERROR mode");
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    default:
        DEVICE_STATE = old_state;
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}


void controller_task(void *pvParameters){

    // Get the queues from the pvParameters pointer
    QueueHandle_t *queues = (QueueHandle_t *)pvParameters;
    QueueHandle_t measurements_queue = queues[0];
    QueueHandle_t led_state_queue = queues[1];
    QueueHandle_t errors_queue = queues[2];

    // If the led state queue failed at being created, we go into an infinite loop
    if (led_state_queue == 0){
        ESP_LOGD(TAG, "Led state queue is 0, entering infinite loop");
        while (1){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // If any of the other queues failed at being created, the device is set to an unrecoverable ERROR mode
    if ((measurements_queue == 0) || (errors_queue == 0)){set_device_state(ERROR, led_state_queue);}

    set_device_state(BOOTING, led_state_queue);

    //Begin the infinite loop
    struct SCD40measurement meas;
    esp_err_t err;
    while (1){
        // Check for errors
        if (xQueueReceive(errors_queue, &( err), (TickType_t) 10)){
            set_device_state(ERROR, led_state_queue);
        }

        // Check for a new measurement
        if (xQueueReceive(measurements_queue, &( meas), (TickType_t) 10)){
            handle_measurement(meas, led_state_queue);
        }


    }
}

// #ifdef __cplusplus
// }
// #endif