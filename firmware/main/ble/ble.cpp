#include "advertisement.h"
#include "constants.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_defs.h"
#include "esp_gattc_api.h"
#include <esp_log.h>
#include <esp_err.h>
#include <esp_check.h>
#include "esp_sleep.h"
#include "freertos/task.h"
#include "measurement.h"
#include "nvs_flash.h"

#include <cstdint>
#include <cstring>

// Project files
#include "../types.h"

static const char *BLE_TAG = "ble";


constexpr uint8_t BIND_KEY[] = {0x23, 0x1d, 0x39, 0xc1, 0xd7, 0xcc, 0x1a, 0xb1,
                                0xae, 0xe2, 0x24, 0xcd, 0x09, 0x6d, 0xb9, 0x32};


static esp_ble_adv_params_t ble_adv_params = {
    .adv_int_min       = 0x20,
    .adv_int_max       = 0x40,
    .adv_type          = ADV_TYPE_NONCONN_IND,
    .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .peer_addr         = 0,
    .peer_addr_type    = BLE_ADDR_TYPE_PUBLIC,
    .channel_map       = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

esp_err_t ble_init(void)
{
    uint8_t N_init_tasks = 6;
    ESP_LOGI(BLE_TAG, "Initializing BLE...");
    ESP_LOGI(BLE_TAG, "1/%u Initializing NVS flash", N_init_tasks);
    ESP_RETURN_ON_ERROR(nvs_flash_init(), BLE_TAG, "NVS flash init failed");
    ESP_LOGI(BLE_TAG, "2/%u Releasing controller memory", N_init_tasks);
    ESP_RETURN_ON_ERROR(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT), BLE_TAG, "Releasing controller memory failed");

    ESP_LOGI(BLE_TAG, "3/%u Initializing BT controller", N_init_tasks);
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_bt_controller_init(&bt_cfg), BLE_TAG, "BT controller init failed");

    ESP_LOGI(BLE_TAG, "4/%u Enabling BT controller", N_init_tasks);
    ESP_RETURN_ON_ERROR(esp_bt_controller_enable(ESP_BT_MODE_BLE), BLE_TAG, "Enabling BT controller failed");

    ESP_LOGI(BLE_TAG, "5/%u Initializing bluedroid", N_init_tasks);
    ESP_RETURN_ON_ERROR(esp_bluedroid_init(), BLE_TAG, "Bluedroid init failed");
    ESP_LOGI(BLE_TAG, "6/%u Enabling bluedroid", N_init_tasks);
    ESP_RETURN_ON_ERROR(esp_bluedroid_enable(), BLE_TAG, "Enabling bluedroid failed");

    ESP_LOGI(BLE_TAG, "BLE initialized successfully");
    return ESP_OK;
}

void ble_deinit(void)
{
    ESP_LOGI(BLE_TAG, "Deinitializing BLE...");
    ESP_ERROR_CHECK(esp_bluedroid_disable());
    ESP_ERROR_CHECK(esp_bluedroid_deinit());
    ESP_ERROR_CHECK(esp_bt_controller_disable());
    ESP_ERROR_CHECK(esp_bt_controller_deinit());
    ESP_LOGI(BLE_TAG, "BLE deinitialized sucesfully");
}

uint8_t build_data_advert(uint8_t data[], SCD40measurement meas)
{
    bthome::Measurement temp(bthome::constants::ObjectId::TEMPERATURE_PRECISE, meas.temperature);
    bthome::Measurement humid(bthome::constants::ObjectId::HUMIDITY_PRECISE, meas.humidity);
    bthome::Measurement co2(bthome::constants::ObjectId::CO2, (uint64_t)meas.co2);

    bthome::Advertisement advertisement("MINICO2", false, BIND_KEY);
    advertisement.addMeasurement(temp);
    advertisement.addMeasurement(humid);
    advertisement.addMeasurement(co2);

    memcpy(&data[0], advertisement.getPayload(), advertisement.getPayloadSize());

    return advertisement.getPayloadSize();
}

void ble_task(void* pvParameters)
{
    // Get the queues from the pvParameters pointer
    QueueHandle_t *queues = (QueueHandle_t *)pvParameters;
    QueueHandle_t ble_queue = queues[0];
    QueueHandle_t errors_queue = queues[1];

    // If any of the queues failed at being created, we go into an infinite loop
    if ((ble_queue == 0) || (errors_queue == 0)){
        ESP_LOGD(BLE_TAG, "BLE queue or errors queue is 0, entering infinite loop");
        while (1){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Init the BLE
    esp_err_t ble_init_err = ble_init();
    if (ble_init_err){
        // Log the error, put it on the errors queue, and enter an infinite loop
        ESP_ERROR_CHECK_WITHOUT_ABORT(ble_init_err);
        xQueueSendToBack(errors_queue, &ble_init_err, (TickType_t)0);
        while (1){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);


    struct SCD40measurement meas;
    while(1){
        // Wait for sensor data to be received
        if (xQueueReceive(ble_queue, &( meas), (TickType_t) 10)){
            ESP_LOGI(BLE_TAG, "Sensor data received on queue");

            // Encode sensor data
            static uint8_t advertData[64];
            uint8_t const dataLength = build_data_advert(&advertData[0], meas);

            if (dataLength > bthome::constants::BLE_ADVERT_MAX_LEN){
                ESP_LOGE(BLE_TAG, "Advert size %i is too big, can't send it", dataLength);
            }
            else{
                ESP_LOGI(BLE_TAG, "Advert size: %i bytes", dataLength);
                // Configure advertising data
                ESP_ERROR_CHECK(esp_ble_gap_config_adv_data_raw(&advertData[0], dataLength));

                // Begin advertising
                ESP_ERROR_CHECK(esp_ble_gap_start_advertising(&ble_adv_params));

                // Wait 1500ms for a few advertisement to go out
                // The minimum time is 1s, the maximum time is 1.28s, so waiting
                // 320ms beyond that
                vTaskDelay(1500 / portTICK_PERIOD_MS);

                // Stop advertising data
                ESP_ERROR_CHECK(esp_ble_gap_stop_advertising());
            }
        }
    }
}