#ifndef _ZIGBEE_H
#define _ZIGBEE_H

#include "esp_zigbee_core.h"

void zigbee_task(void *pvParameters);

/* Zigbee configuration */
#define INSTALLCODE_POLICY_ENABLE       false   /* enable the install code policy for security */
#define ED_AGING_TIMEOUT                ESP_ZB_ED_AGING_TIMEOUT_4MIN /* Not sure what this is. */
#define ED_KEEP_ALIVE                   3000    /* Time period after which the MINICO2 must send a signal to its parent device to confirm it is still active on the network */
#define HA_ESP_SENSOR_ENDPOINT          10      /* Home Assistance device endpoint, used for temperature and humidity measurement */
#define ESP_ZB_PRIMARY_CHANNEL_MASK     ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK    /* Zigbee primary channel mask use in the example */

#define ESP_TEMP_SENSOR_MIN_VALUE       (-10)   /* SCD4x temp sensor min measured value (degree Celsius)   */
#define ESP_TEMP_SENSOR_MAX_VALUE       (60)    /* SCD4x temp sensor max measured value (degree Celsius)   */
#define ESP_RH_SENSOR_MIN_VALUE         (0)     /* SCD4x relative humidity sensor min measured value (%)   */
#define ESP_RH_SENSOR_MAX_VALUE         (100)   /* SCD4x relative humidity max measured value (%)          */
#define ESP_CO2_SENSOR_MIN_VALUE        (0*1E-6)    /* SCD4x CO2 sensor min measured value (fraction of 1) */
#define ESP_CO2_SENSOR_MAX_VALUE        (2000*1E-6) /* SCD4x CO2 sensor max measured value (fraction of 1) */

/* Attribute values in ZCL string format
 * The string should be started with the length of its own.
 */
#define MANUFACTURER_NAME               "\x07""MAGLAUR"
#define MODEL_IDENTIFIER                "\x07""MINICO2"

#define ESP_ZB_ZED_CONFIG()                                         \
    {                                                               \
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED,                       \
        .install_code_policy = INSTALLCODE_POLICY_ENABLE,           \
        .nwk_cfg.zed_cfg = {                                        \
            .ed_timeout = ED_AGING_TIMEOUT,                         \
            .keep_alive = ED_KEEP_ALIVE,                            \
        },                                                          \
    }

#define ESP_ZB_DEFAULT_RADIO_CONFIG()                           \
    {                                                           \
        .radio_mode = ZB_RADIO_MODE_NATIVE,                     \
    }

#define ESP_ZB_DEFAULT_HOST_CONFIG()                            \
    {                                                           \
        .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE,   \
    }

/* Custom types */

/**
 * @brief Zigbee HA MiniCO2 clusters.
 *
 */
typedef struct esp_zb_minico2_cfg_s {
    esp_zb_basic_cluster_cfg_t basic_cfg;                           /*!<  Basic cluster configuration, @ref esp_zb_basic_cluster_cfg_s */
    esp_zb_identify_cluster_cfg_t identify_cfg;                     /*!<  Identify cluster configuration, @ref esp_zb_identify_cluster_cfg_s */
    esp_zb_temperature_meas_cluster_cfg_t temp_meas_cfg;            /*!<  Temperature measurement cluster configuration, @ref esp_zb_temperature_meas_cluster_cfg_s */
    esp_zb_humidity_meas_cluster_cfg_t hum_meas_cfg;                /*!<  Humidity measurement cluster configuration, @ref esp_zb_humidity_meas_cluster_cfg_s */
    esp_zb_carbon_dioxide_measurement_cluster_cfg_t co2_meas_cfg;   /*!<  Carbon dioxide measurement cluster configuration, @ref esp_zb_carbon_dioxide_measurement_cluster_cfg_s */
} esp_zb_minico2_cfg_t;


/**
 * @brief Zigbee HA standard MiniCO2 device default config value.
 *
 */
#define ESP_ZB_DEFAULT_MINICO2_SENSOR_CONFIG()                                                      \
    {                                                                                               \
        .basic_cfg =                                                                                \
            {                                                                                       \
                .zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,                          \
                .power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_DEFAULT_VALUE,                        \
            },                                                                                      \
        .identify_cfg =                                                                             \
            {                                                                                       \
                .identify_time = ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE,                   \
            },                                                                                      \
        .temp_meas_cfg =                                                                            \
            {                                                                                       \
                .measured_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MEASURED_VALUE_DEFAULT,               \
                .min_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MIN_MEASURED_VALUE_DEFAULT,                \
                .max_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MAX_MEASURED_VALUE_DEFAULT,                \
            },                                                                                      \
        .hum_meas_cfg =                                                                             \
            {                                                                                       \
                .measured_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MEASURED_VALUE_DEFAULT,               \
                .min_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MIN_MEASURED_VALUE_DEFAULT,                \
                .max_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MAX_MEASURED_VALUE_DEFAULT,                \
            },                                                                                      \
        .co2_meas_cfg =                                                                             \
            {                                                                                       \
                .measured_value = ESP_ZB_ZCL_CARBON_DIOXIDE_MEASUREMENT_MEASURED_VALUE_DEFAULT,     \
                .min_measured_value = ESP_ZB_ZCL_CARBON_DIOXIDE_MEASUREMENT_MIN_MEASURED_VALUE_DEFAULT,      \
                .max_measured_value = ESP_ZB_ZCL_CARBON_DIOXIDE_MEASUREMENT_MIN_MEASURED_VALUE_DEFAULT,      \
            },                                                                                      \
    }
#endif