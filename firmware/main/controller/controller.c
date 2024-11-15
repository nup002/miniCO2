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
    ESP_LOGI(TAG, "Received a sensor measurement");
    ESP_LOGI(TAG, "CO2: %u ppm", meas.co2);
    ESP_LOGI(TAG, "Temperature: %.2f °C", meas.temperature);
    ESP_LOGI(TAG, "Humidity: %.2f %%", meas.humidity);

    set_led_state_from_co2(meas.co2, led_state_queue);
}


void controller_task(void *pvParameters){

    // Get the queues from the pvParameters pointer
    QueueHandle_t *queues = (QueueHandle_t *)pvParameters;
    QueueHandle_t measurements_queue = queues[0];
    QueueHandle_t led_state_queue = queues[1];

    //Begin the infinite loop
    struct SCD40measurement meas;
    while (1){

        // Check for a new measurement
        if (xQueueReceive(measurements_queue, &( meas), (TickType_t) 10)){
            handle_measurement(meas, led_state_queue);
        }


    }
}
