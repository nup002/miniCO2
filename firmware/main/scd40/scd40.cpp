#include "scd40.h"
#include "scd4x.h"
#include <esp_err.h>
#include <esp_check.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <freertos/queue.h>
#include "../types.h"

#define SELF_TEST_SENSOR false

static constexpr const char *SCD40_TAG = "scd40";
static int MEASURE_INTERVAL = 10;

i2c_dev_t SCD40DEV;

esp_err_t init_scd40(void)
{
    uint8_t N_init_tasks = 6;

    ESP_RETURN_ON_ERROR(scd4x_init_desc(&SCD40DEV, (i2c_port_t)0, SCD40_SDA, SCD40_SCL), SCD40_TAG, "SCD40 descriptor init failed");

    ESP_LOGI(SCD40_TAG, "Initializing sensor...");
    ESP_LOGI(SCD40_TAG, "1/%u - Waking up sensor", N_init_tasks);
    scd4x_wake_up(&SCD40DEV); // Raises a false positive error, so we don't error check it

    ESP_LOGI(SCD40_TAG, "2/%u - Stopping periodic sensor measurements", N_init_tasks);
    ESP_RETURN_ON_ERROR(scd4x_stop_periodic_measurement(&SCD40DEV), SCD40_TAG, "SCD40 stop periodic measurements failed");

    ESP_LOGI(SCD40_TAG, "3/%u - Reinitializing sensor", N_init_tasks);
    ESP_RETURN_ON_ERROR(scd4x_reinit(&SCD40DEV), SCD40_TAG, "SCD40 reinitialization failed");
    
    if (SELF_TEST_SENSOR)
    {
        bool malfunction;
        ESP_LOGI(SCD40_TAG, "4/%u - Performing sensor self-test", N_init_tasks);
        ESP_RETURN_ON_ERROR(scd4x_perform_self_test(&SCD40DEV, &malfunction), SCD40_TAG, "SCD40 self test failed");
        if (!malfunction)
        {
            ESP_LOGI(SCD40_TAG, "Sensor self-test success");
        } else {
            ESP_LOGE(SCD40_TAG, "Sensor self-test failure");
            return ESP_FAIL;
        }
    }

    uint16_t serial[3];
    ESP_LOGI(SCD40_TAG, "5/%u - Getting sensor serial number", N_init_tasks);
    ESP_RETURN_ON_ERROR(scd4x_get_serial_number(&SCD40DEV, serial, serial + 1, serial + 2), SCD40_TAG, "SCD40 get serial number failed");
    ESP_LOGI(SCD40_TAG, "Sensor serial number: 0x%04x%04x%04x", serial[0], serial[1], serial[2]);

    ESP_LOGI(SCD40_TAG, "6/%u - Disabling automatic sensor self-calibration", N_init_tasks);
    ESP_RETURN_ON_ERROR(scd4x_set_automatic_self_calibration(&SCD40DEV, false), SCD40_TAG, "SCD40 disabling of automatic self calibration failed");
    
    ESP_LOGI(SCD40_TAG, "Sensor initialized succesfully");
    return ESP_OK;
}

void scd40_task(void *pvParameters)
{
    // Get the queues from the pvParameters pointer
    QueueHandle_t *queues = (QueueHandle_t *)pvParameters;
    QueueHandle_t measurements_queue = queues[0];
    QueueHandle_t errors_queue = queues[1];

    // If any of the queues failed at being created, we go into an infinite loop
    if ((measurements_queue == 0) || (errors_queue == 0)){
        ESP_LOGE(SCD40_TAG, "Measurements queue or errors queue is 0, entering infinite loop");
        while (1){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Init the sensor
    esp_err_t scd40_init_err = init_scd40();
    if (scd40_init_err){
        // Log the error, put it on the errors queue, and enter an infinite loop
        ESP_ERROR_CHECK_WITHOUT_ABORT(scd40_init_err);
        xQueueSendToBack(errors_queue, &scd40_init_err, (TickType_t)0);
        while (1){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Begin infinite loop of taking measurements 
    static int64_t time0 = 0;
    struct SCD40measurement meas;
    bool data_ready;
    while (1)
    {
        if ((esp_timer_get_time() - time0)/1E6 > MEASURE_INTERVAL - 0.25){
            time0 = esp_timer_get_time();
            scd4x_measure_single_shot(&SCD40DEV);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        scd4x_get_data_ready_status(&SCD40DEV, &data_ready);
        if (!data_ready){continue;}
        
        esp_err_t res = scd4x_read_measurement(&SCD40DEV, &meas.co2, &meas.temperature, &meas.humidity);
        if (res != ESP_OK)
        {
            ESP_LOGE(SCD40_TAG, "Error reading results %d (%s)", res, esp_err_to_name(res));
            continue;
        }

        if (meas.co2 == 0)
        {
            ESP_LOGW(SCD40_TAG, "Invalid sample detected, skipping");
            continue;
        }

        ESP_LOGD(SCD40_TAG, "Sending measurement on the queue");
        xQueueSendToBack(measurements_queue, &meas, (TickType_t)0);
    }
}
