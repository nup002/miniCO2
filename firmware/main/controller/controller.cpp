#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "driver/gpio.h"
#include <esp_err.h>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_event.h>
#include "sdkconfig.h"
#include "../types.h"
extern "C" {
#include "../console/console.h"
#include "../globals.h"
#include "../config/config.h"
}

static const char *CONTROLLER_TAG = "MINICO2";
static enum DEVICE_STATES DEVICE_STATE = BOOTING; 
struct SCD40measurement most_recent_measurement = {0};


void handle_measurement(struct SCD40measurement meas, QueueHandle_t ble_queue, QueueHandle_t zigbee_queue){
    // Print the measurement in JSON on the serial connection
    if (MINICO2CONFIG.serial_print_enabled) {
        printf("{CO2: %u, TEMP: %.1f, HUM: %.1f}\n", meas.co2, meas.temperature, meas.humidity);
    }
    most_recent_measurement = meas;

    // Send the measurement to the BLE task for transmission
    xQueueSendToBack(ble_queue, &meas, (TickType_t)0);

    // Send the measurement to the zigbee task for transmission
    xQueueSendToBack(zigbee_queue, &meas, (TickType_t)0);
}

// Handler for changes to the LED CO2 limits 
static void co2_limits_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
    // TODO: Implement screen update
    return;
}

esp_err_t set_device_state(enum DEVICE_STATES state){
    enum DEVICE_STATES old_state = DEVICE_STATE;
    DEVICE_STATE = state;
    ESP_LOGD(CONTROLLER_TAG, "Device state changing from %s to %s", xstr(old_state), xstr(DEVICE_STATE));
    switch (DEVICE_STATE)
    {
    case BOOTING:
        break;
    case ERROR:
        while (1){
            ESP_LOGE(CONTROLLER_TAG, "MINICO2 is in ERROR mode");
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
    QueueHandle_t errors_queue = queues[1];
    QueueHandle_t ble_queue = queues[2];
    QueueHandle_t zigbee_queue = queues[3];

    // If any of the queues failed at being created, the device is set to an unrecoverable ERROR mode
    if ((measurements_queue == 0) || (errors_queue == 0) || (ble_queue == 0)){set_device_state(ERROR);}

    set_device_state(BOOTING);

    // Connect event handlers
    //ESP_ERROR_CHECK(esp_event_handler_instance_register(CONFIG_EVENTS, CO2_LIMITS_EVENT, co2_limits_handler, NULL));

    // Give the device time to boot and then launch the console
    vTaskDelay(pdMS_TO_TICKS(1000));
    start_console();

    //Begin the infinite loop
    struct SCD40measurement meas;
    esp_err_t err;
    while (1){
        // Check for errors
        if (xQueueReceive(errors_queue, &( err), (TickType_t) 10)){
            set_device_state(ERROR);
        }

        // Check for a new measurement
        if (xQueueReceive(measurements_queue, &( meas), (TickType_t) 10)){
            handle_measurement(meas, ble_queue, zigbee_queue);
        }

    }
}

// #ifdef __cplusplus
// }
// #endif