#include "advertisement.h"
#include "constants.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_defs.h"
#include "esp_gattc_api.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/task.h"
#include "measurement.h"
#include "nvs_flash.h"

#include <cstdint>
#include <cstring>

#define BLE_TASK_NAME    "ble"


constexpr uint8_t BIND_KEY[] = {0x23, 0x1d, 0x39, 0xc1, 0xd7, 0xcc, 0x1a, 0xb1,
                                0xae, 0xe2, 0x24, 0xcd, 0x09, 0x6d, 0xb9, 0x32};
                                
static constexpr uint32_t US_TO_S_FACTOR {1000000};
static constexpr uint32_t SECONDS_PER_MINUTE {60};
static constexpr uint64_t SLEEP_1_MINUTE {SECONDS_PER_MINUTE * US_TO_S_FACTOR};
static constexpr uint64_t SLEEP_5_MINUTES {SLEEP_1_MINUTE * 5};

static bthome::Advertisement advertisement("MINICO2", false, BIND_KEY);

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

static uint8_t advertData[64];

void ble_init(void)
{
    ESP_LOGI(BLE_TASK_NAME, "Starting BLE init");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));

    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_LOGI(BLE_TASK_NAME, "BLE init completed");
}

void ble_deinit(void)
{
    ESP_ERROR_CHECK(esp_bluedroid_disable());
    ESP_ERROR_CHECK(esp_bluedroid_deinit());
    ESP_ERROR_CHECK(esp_bt_controller_disable());
    ESP_ERROR_CHECK(esp_bt_controller_deinit());
}

uint8_t build_data_advert(uint8_t data[])
{
    // bthome::Measurement temp(bthome::constants::ObjectId::TEMPERATURE_PRECISE, blackboard.sensors.temperature);
    // bthome::Measurement humid(bthome::constants::ObjectId::HUMIDITY_PRECISE, blackboard.sensors.humidity);
    // bthome::Measurement press(bthome::constants::ObjectId::PRESSURE, blackboard.sensors.pressure);

    // advertisement.addMeasurement(temp);
    // advertisement.addMeasurement(humid);
    // advertisement.addMeasurement(press);

    // memcpy(&data[0], advertisement.getPayload(), advertisement.getPayloadSize());

    // return advertisement.getPayloadSize();
    return 0;
}

void task_ble_entry(void* params)
{
    ble_init();

    vTaskDelay(200 / portTICK_PERIOD_MS);

    ESP_LOGI(BLE_TASK_NAME, "Entering task loop");

    for (;;)
    {
        // Wait for sensor task notify it has read data
        if (pdTRUE == ulTaskNotifyTake(pdTRUE, 1000 / portTICK_PERIOD_MS))
        {

            ESP_LOGI(BLE_TASK_NAME, "Sensor data ready");

            // Encode sensor data
            uint8_t const dataLength = build_data_advert(&advertData[0]);

            if (dataLength > bthome::constants::BLE_ADVERT_MAX_LEN)
            {
                ESP_LOGE(BLE_TASK_NAME, "Advert size %i is too big, can't send it", dataLength);
            }
            else
            {
                ESP_LOGI(BLE_TASK_NAME, "Advert size: %i bytes", dataLength);
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

            // De-init all BLE related things
            ble_deinit();

            // Enter deep sleep
            ESP_LOGI(BLE_TASK_NAME, "Goodbye!");
            esp_deep_sleep(SLEEP_5_MINUTES);
        }
        else
        {
            ESP_LOGE(BLE_TASK_NAME, "Timed out waiting for sensor data");
            // Handle the timeout somehow
        }
    }
}