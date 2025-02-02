/* 
This file contains the ZigBee functionality of the MiniCO2. This was my first time doing anything related to ZigBee. 
Apologies in advance for the likely blunders. Do you know ZigBee? Feel free to make improvements.
*/

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "zigbee.h"
#include "../types.h"

static const char *ZIGBEE_TAG = "zigbee";

static int16_t zb_temp_and_hum_to_s16(float temp_or_hum)
{
    return (int16_t)(temp_or_hum * 100);
}

static float_t zb_co2_to_float(uint16_t co2)
{
    return (float_t)(co2 * 1E-6);
}

// static void esp_app_buttons_handler(switch_func_pair_t *button_func_pair)
// {
//     if (button_func_pair->func == SWITCH_ONOFF_TOGGLE_CONTROL) {
//         /* Send report attributes command */
//         esp_zb_zcl_report_attr_cmd_t report_attr_cmd = {0};
//         report_attr_cmd.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
//         report_attr_cmd.attributeID = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID;
//         report_attr_cmd.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
//         report_attr_cmd.clusterID = ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT;
//         report_attr_cmd.zcl_basic_cmd.src_endpoint = HA_ESP_SENSOR_ENDPOINT;

//         esp_zb_lock_acquire(portMAX_DELAY);
//         esp_zb_zcl_report_attr_cmd_req(&report_attr_cmd);
//         esp_zb_lock_release();
//         ESP_EARLY_LOGI(ZIGBEE_TAG, "Send 'report attributes' command");
//     }
// }

static void esp_app_measurement_handler(struct SCD40measurement measurement)
{
    int16_t temp = zb_temp_and_hum_to_s16(measurement.temperature);
    int16_t hum = zb_temp_and_hum_to_s16(measurement.humidity);
    float_t co2 = zb_co2_to_float(measurement.co2);
    esp_zb_lock_acquire(portMAX_DELAY);
    /* Update temperature sensor measured value */
    esp_zb_zcl_set_attribute_val(HA_ESP_SENSOR_ENDPOINT,
        ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, &temp, false);
    /* Update humidity sensor measured value */
    esp_zb_zcl_set_attribute_val(HA_ESP_SENSOR_ENDPOINT,
        ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID, &hum, false);
    /* Update co2 sensor measured value */
    esp_zb_zcl_set_attribute_val(HA_ESP_SENSOR_ENDPOINT,
        ESP_ZB_ZCL_CLUSTER_ID_CARBON_DIOXIDE_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_CARBON_DIOXIDE_MEASUREMENT_MEASURED_VALUE_ID, &co2, false);
    esp_zb_lock_release();
}

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ESP_RETURN_ON_FALSE(esp_zb_bdb_start_top_level_commissioning(mode_mask) == ESP_OK, ,
                        ZIGBEE_TAG, "Failed to start Zigbee bdb commissioning");
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p     = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = *p_sg_p;
    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(ZIGBEE_TAG, "Initialize Zigbee stack");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(ZIGBEE_TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(ZIGBEE_TAG, "Start network steering");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            } else {
                ESP_LOGI(ZIGBEE_TAG, "Device rebooted");
            }
        } else {
            /* commissioning failed */
            ESP_LOGW(ZIGBEE_TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
        }
        break;
    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(ZIGBEE_TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                     extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                     esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
        } else {
            ESP_LOGD(ZIGBEE_TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 5000);
        }
        break;
    default:
        ESP_LOGI(ZIGBEE_TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
                 esp_err_to_name(err_status));
        break;
    }
}



/* Create the clusters for the Home Assistant endpoint */
static esp_zb_cluster_list_t *custom_minico2_clusters_create(esp_zb_minico2_cfg_t *minico2_sensor)
{
    /* Get an empty cluster list */
    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
    /* Create basic cluster with basic config */
    esp_zb_attribute_list_t *basic_cluster = esp_zb_basic_cluster_create(&(minico2_sensor->basic_cfg));

    /* Add manufacturer name and model name to the basic cluster */
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, MANUFACTURER_NAME));
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, MODEL_IDENTIFIER));

    /* Add the basic information cluster to the cluster list */
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_basic_cluster(cluster_list, basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    /* Add indentify cluster to the cluster list */
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_identify_cluster(cluster_list, esp_zb_identify_cluster_create(&(minico2_sensor->identify_cfg)), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    /* No idea what's going on here */
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_identify_cluster(cluster_list, esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY), ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE));

    /* Add the sensor clusters to the cluster list */
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_temperature_meas_cluster(cluster_list, esp_zb_temperature_meas_cluster_create(&(minico2_sensor->temp_meas_cfg)), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_humidity_meas_cluster(cluster_list, esp_zb_humidity_meas_cluster_create(&(minico2_sensor->hum_meas_cfg)), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_carbon_dioxide_measurement_cluster(cluster_list, esp_zb_carbon_dioxide_measurement_cluster_create(&(minico2_sensor->co2_meas_cfg)), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    /* Add the LED configuration clusters to the cluster list */
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_on_off_cluster(cluster_list, esp_zb_on_off_cluster_create(&(minico2_sensor->led_on_off_cfg)), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_level_cluster(cluster_list, esp_zb_level_cluster_create(&(minico2_sensor->led_brightness_cfg)), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    return cluster_list;
}

/* Create the MINICO2 ZigBee Home Assistant endpoint. */
static esp_zb_ep_list_t *custom_minico2_ha_ep_create(uint8_t endpoint_id, esp_zb_minico2_cfg_t *minico2_sensor)
{
    /* Get the list of existing endpoints. */
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();

    /* Create the config for the HA endpoint */
    esp_zb_endpoint_config_t ha_endpoint_config = {
        .endpoint = endpoint_id,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_SIMPLE_SENSOR_DEVICE_ID,  // There may be a more suitable device ID than this
        .app_device_version = 0
    };

    /* Create the list of clusters for the Home Assistant endpoint*/
    esp_zb_cluster_list_t *clusters = custom_minico2_clusters_create(minico2_sensor);

    /* Add the list of clusters to a new endpoint defined by ha_endpoint_config, and add that endpoint to the list of all endpoints */
    esp_zb_ep_list_add_ep(ep_list, clusters, ha_endpoint_config);
    return ep_list;
}

static void zigbee_setup()
{
    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    /* Initialize Zigbee stack */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    esp_zb_init(&zb_nwk_cfg);

    /* 
    Create customized MINICO2 endpoint 
    */
    esp_zb_minico2_cfg_t minico2_cfg = ESP_ZB_DEFAULT_MINICO2_SENSOR_CONFIG();

    /* Set sensor limits */
    minico2_cfg.temp_meas_cfg.min_value = zb_temp_and_hum_to_s16(ESP_TEMP_SENSOR_MIN_VALUE);
    minico2_cfg.temp_meas_cfg.max_value = zb_temp_and_hum_to_s16(ESP_TEMP_SENSOR_MAX_VALUE);
    minico2_cfg.hum_meas_cfg.min_value = zb_temp_and_hum_to_s16(ESP_RH_SENSOR_MIN_VALUE);
    minico2_cfg.hum_meas_cfg.max_value = zb_temp_and_hum_to_s16(ESP_RH_SENSOR_MAX_VALUE);
    minico2_cfg.co2_meas_cfg.min_measured_value = zb_temp_and_hum_to_s16(ESP_RH_SENSOR_MIN_VALUE);
    minico2_cfg.co2_meas_cfg.max_measured_value = zb_temp_and_hum_to_s16(ESP_RH_SENSOR_MAX_VALUE);

    /* Build the Home Assistant endpoint */
    esp_zb_ep_list_t *esp_zb_sensor_ep = custom_minico2_ha_ep_create(HA_ESP_SENSOR_ENDPOINT, &minico2_cfg);

    /* Register the Home Assistant endpoint. This is like 'committing' the endpoint, after which we cannot modify it. */
    esp_zb_device_register(esp_zb_sensor_ep);

    /* Config the reporting info for temperature */
    esp_zb_zcl_reporting_info_t temp_reporting_info = {
        .direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV,
        .ep = HA_ESP_SENSOR_ENDPOINT,
        .cluster_id = ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
        .cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        .dst.profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .u.send_info.min_interval = 1,
        .u.send_info.max_interval = 0,
        .u.send_info.def_min_interval = 1,
        .u.send_info.def_max_interval = 0,
        .u.send_info.delta.u16 = 100,
        .attr_id = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
        .manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
    };
    esp_zb_zcl_update_reporting_info(&temp_reporting_info);

    /* Config the reporting info for humidity */
    esp_zb_zcl_reporting_info_t rh_reporting_info = {
        .direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV,
        .ep = HA_ESP_SENSOR_ENDPOINT,
        .cluster_id = ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
        .cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        .dst.profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .u.send_info.min_interval = 1,
        .u.send_info.max_interval = 0,
        .u.send_info.def_min_interval = 1,
        .u.send_info.def_max_interval = 0,
        .u.send_info.delta.u16 = 100,
        .attr_id = ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
        .manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
    };
    esp_zb_zcl_update_reporting_info(&rh_reporting_info);

    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));
}

void zigbee_data_handler_task(void *pvParameters)
{
    // Get the queues from the pvParameters pointer
    QueueHandle_t *queues = (QueueHandle_t *)pvParameters;
    QueueHandle_t zigbee_queue = queues[0];
    QueueHandle_t errors_queue = queues[1];

    // If any of the queues failed at being created, we go into an infinite loop
    if ((zigbee_queue == 0) || (errors_queue == 0)){
        ESP_LOGE(ZIGBEE_TAG, "Zigbee queue or errors queue is 0, entering infinite loop");
        while (1){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    ESP_LOGI(ZIGBEE_TAG, "Zigbee data handler task launched.");
    struct SCD40measurement meas;
    while (1){
        if (xQueueReceive(zigbee_queue, &( meas), (TickType_t) 10)){
            ESP_LOGD(ZIGBEE_TAG, "Sensor data received on Zigbee queue");
            esp_app_measurement_handler(meas);
        }
    }
}

void zigbee_task(void *pvParameters)
{
    // Get the queues from the pvParameters pointer
    QueueHandle_t *queues = (QueueHandle_t *)pvParameters;
    QueueHandle_t zigbee_queue = queues[0];
    QueueHandle_t errors_queue = queues[1];

    // If any of the queues failed at being created, we go into an infinite loop
    if ((zigbee_queue == 0) || (errors_queue == 0)){
        ESP_LOGE(ZIGBEE_TAG, "Zigbee queue or errors queue is 0, entering infinite loop");
        while (1){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    /* Setup zigbee */
    zigbee_setup();

    /* Launch the ZigBee data handler task */
    TaskHandle_t zigbee_data_task_handle = NULL;
    QueueHandle_t zigbee_queues[] = {zigbee_queue, errors_queue}; 
    xTaskCreate(zigbee_data_handler_task, "Zigbee_data_task", configMINIMAL_STACK_SIZE * 8, zigbee_queues, 5, &zigbee_data_task_handle);

    ESP_LOGI(ZIGBEE_TAG, "Zigbee setup finished, launching zigbee stack main loop.");
    esp_zb_stack_main_loop();
}