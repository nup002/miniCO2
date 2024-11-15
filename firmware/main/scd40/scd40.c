#include "scd40.h"
#include "scd4x.h"
#include <esp_err.h>
#include <esp_check.h>
#include <esp_log.h>
#include <freertos/queue.h>
#include "../types.h"

#define SELF_TEST_SENSOR false

static const char *TAG = "scd40";


i2c_dev_t SCD40DEV = { 0 };

esp_err_t init_scd40(void)
{
    uint8_t N_init_tasks = 7;

    ESP_RETURN_ON_ERROR(scd4x_init_desc(&SCD40DEV, 0, SCD40_SDA, SCD40_SCL), TAG, "SCD40 descriptor init failed");

    ESP_LOGI(TAG, "Initializing sensor...");
    ESP_LOGI(TAG, "1/%u - Waking up sensor", N_init_tasks);
    scd4x_wake_up(&SCD40DEV); // Raises a false positive error, so we don't error check it

    ESP_LOGI(TAG, "2/%u - Stopping periodic sensor measurements", N_init_tasks);
    ESP_RETURN_ON_ERROR(scd4x_stop_periodic_measurement(&SCD40DEV), TAG, "SCD40 stop periodic measurements failed");

    ESP_LOGI(TAG, "3/%u - Reinitializing sensor", N_init_tasks);
    ESP_RETURN_ON_ERROR(scd4x_reinit(&SCD40DEV), TAG, "SCD40 reinitialization failed");
    
    if (SELF_TEST_SENSOR)
    {
        bool malfunction;
        ESP_LOGI(TAG, "4/%u - Performing sensor self-test", N_init_tasks);
        ESP_RETURN_ON_ERROR(scd4x_perform_self_test(&SCD40DEV, &malfunction), TAG, "SCD40 self test failed");
        if (!malfunction)
        {
            ESP_LOGI(TAG, "Sensor self-test success");
        } else {
            ESP_LOGE(TAG, "Sensor self-test failure");
            return ESP_FAIL;
        }
    }

    uint16_t serial[3];
    ESP_LOGI(TAG, "5/%u - Getting sensor serial number", N_init_tasks);
    ESP_RETURN_ON_ERROR(scd4x_get_serial_number(&SCD40DEV, serial, serial + 1, serial + 2), TAG, "SCD40 get serial number failed");
    ESP_LOGI(TAG, "Sensor serial number: 0x%04x%04x%04x", serial[0], serial[1], serial[2]);

    ESP_LOGI(TAG, "6/%u - Disabling automatic sensor self-calibration", N_init_tasks);
    ESP_RETURN_ON_ERROR(scd4x_set_automatic_self_calibration(&SCD40DEV, false), TAG, "SCD40 disabling of automatic self calibration failed");

    ESP_LOGI(TAG, "7/%u - Starting periodic sensor measurements", N_init_tasks);
    ESP_RETURN_ON_ERROR(scd4x_start_periodic_measurement(&SCD40DEV), TAG, "SCD40 start of low power periodic measurements failed");
    
    ESP_LOGI(TAG, "Sensor initialized succesfully");
    return ESP_OK;
}

void scd40_task(void *pvParameters)
{
    // Get the measurements queue from the pvParameters pointer
    QueueHandle_t measurements_queue = (QueueHandle_t)pvParameters;

    // Init the sensor
    esp_err_t scd40_init_err = init_scd40();
    if (scd40_init_err){
        // Log the error and put MINICO2 into error state and return from the task
        ESP_ERROR_CHECK_WITHOUT_ABORT(scd40_init_err);
        return;
    }

    struct SCD40measurement meas;
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        bool data_ready;
        scd4x_get_data_ready_status(&SCD40DEV, &data_ready);
        if (!data_ready){continue;}
        
        esp_err_t res = scd4x_read_measurement(&SCD40DEV, &meas.co2, &meas.temperature, &meas.humidity);
        if (res != ESP_OK)
        {
            ESP_LOGE(TAG, "Error reading results %d (%s)", res, esp_err_to_name(res));
            continue;
        }

        if (meas.co2 == 0)
        {
            ESP_LOGW(TAG, "Invalid sample detected, skipping");
            continue;
        }

        ESP_LOGI(TAG, "Sending measurement on the queue.");
        xQueueSendToBack(measurements_queue, &meas, (TickType_t)0);
    }
}
