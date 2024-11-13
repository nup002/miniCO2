#include "scd40.h"
#include "scd4x.h"
#include <esp_err.h>
#include <esp_check.h>
#include <esp_log.h>
#include <freertos/queue.h>
#include "../global_constants.h"


i2c_dev_t SCD40DEV = { 0 };

esp_err_t init_scd40(void)
{
    ESP_RETURN_ON_ERROR(scd4x_init_desc(&SCD40DEV, 0, SCD40_SDA, SCD40_SCL), TAG, "SCD40 descriptor init failed");

    ESP_LOGI(TAG, "Initializing sensor...");
    scd4x_wake_up(&SCD40DEV);
    ESP_RETURN_ON_ERROR(scd4x_stop_periodic_measurement(&SCD40DEV), TAG, "SCD40 stop periodic measurements failed");
    ESP_RETURN_ON_ERROR(scd4x_reinit(&SCD40DEV), TAG, "SCD40 reinitialization failed");
    ESP_LOGI(TAG, "Sensor initialized");
    
    bool malfunction;
    ESP_RETURN_ON_ERROR(scd4x_perform_self_test(&SCD40DEV, &malfunction), TAG, "SCD40 self test failed");
    if (!malfunction)
    {
        ESP_LOGI(TAG, "Sensor self-tested OK");
    } else {
        ESP_LOGE(TAG, "Sensor failed self-test.");
        return ESP_FAIL;
    }

    uint16_t serial[3];
    ESP_RETURN_ON_ERROR(scd4x_get_serial_number(&SCD40DEV, serial, serial + 1, serial + 2), TAG, "SCD40 get serial number failed");
    ESP_LOGI(TAG, "Sensor serial number: 0x%04x%04x%04x", serial[0], serial[1], serial[2]);

    ESP_RETURN_ON_ERROR(scd4x_set_automatic_self_calibration(&SCD40DEV, false), TAG, "SCD40 disabling of automatic self calibration failed");
    ESP_LOGI(TAG, "Automatic self-calibration disabled");

    ESP_RETURN_ON_ERROR(scd4x_start_low_power_periodic_measurement(&SCD40DEV), TAG, "SCD40 start of low power periodic measurements failed");
    ESP_LOGI(TAG, "Low power periodic measurements started");

    return ESP_OK;
}

void scd40_task(void *pvParameters)
{
    // Init the queue
    struct SCD40measurement meas;
    QueueHandle_t queue = xQueueCreate(1, sizeof(meas));
    if (queue == 0)
    {
        ESP_LOGE(TAG, "Failed at creating queue");
    }

    // Init the sensor
    esp_err_t init_err = init_scd40();
    if (init_err){
        // Log the error and put MINICO2 into error state and return from the task
        ESP_ERROR_CHECK_WITHOUT_ABORT(init_err);
        return;
    }


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

        xQueueSendToBack(queue, &meas, (TickType_t)0)

        // ESP_LOGI(TAG, "CO2: %u ppm", meas.co2);
        // ESP_LOGI(TAG, "Temperature: %.2f °C", meas.temperature);
        // ESP_LOGI(TAG, "Humidity: %.2f %%", meas.humidity);
    }
}
