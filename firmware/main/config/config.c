#include <stdbool.h>
#include <string.h>
#include "config.h"
#include "esp_log.h"
#include "../globals.h"
#include "../types.h"

/* 
The MINICO2 configuration must only be modified with the functions defined in this file. 
If the MINICO2CONFIG struct is modified directly, code that relies on its value will not be 
notified of the change. Also, no log statements will be produced.

Every config change is published to the default event loop.
*/

static const char *CONFIG_TAG = "config";

ESP_EVENT_DEFINE_BASE(CONFIG_EVENTS);

/* Enable or disable the printing of sensor measurements to the USB port */
void set_print_sensor_readings(bool enabled){
    MINICO2CONFIG.serial_print_enabled = enabled;
    if (MINICO2CONFIG.serial_print_enabled){
        ESP_LOGI(CONFIG_TAG, "Printing of measurements to the serial port ENABLED");
    }else{
        ESP_LOGI(CONFIG_TAG, "Printing of measurements to the serial port DISABLED");
    }
    ESP_ERROR_CHECK(esp_event_post(CONFIG_EVENTS, PRINT_SENSOR_READINGS_EVENT, NULL, 0, portMAX_DELAY));
}

/* Set the nickname of the MiniCO2 */
void set_nickname(char *nickname){
    if (strlen(nickname) > 128){
        ESP_LOGE(CONFIG_TAG, "Nickname must be shorter than 128 characters.");
        return;
    }

    snprintf(MINICO2CONFIG.name, 128, nickname);
    ESP_LOGI(CONFIG_TAG, "Nickname set to '%s'", MINICO2CONFIG.name);
    ESP_ERROR_CHECK(esp_event_post(CONFIG_EVENTS, NICKNAME_EVENT, NULL, 0, portMAX_DELAY));
}

/* Set the measurement period in seconds. The value is clamped to the range (5 seconds, 1 year). */
void set_measurement_period(int period){
    if (period < 5){period = 5;}
    if (period > 60*60*24*365){period = 60*60*24*365;}
    MINICO2CONFIG.measurement_period = period;
    int p = MINICO2CONFIG.measurement_period;
    ESP_LOGI(CONFIG_TAG, "Measurement period set to %d hours, %d minutes, %d seconds.", p / 3600, (p % 3600) / 60, p % 60);
    ESP_ERROR_CHECK(esp_event_post(CONFIG_EVENTS, MEASUREMENT_PERIOD_EVENT, NULL, 0, portMAX_DELAY));
}

/* Set the LED brightness, from 0 (off) to 1 (brightest). The value is coerced to this range. */
void set_led_brightness(float brightness){
    if (brightness < 0.0){brightness = 0;}
    if (brightness > 1.0){brightness = 1;}
    MINICO2CONFIG.led_cfg.brightness = brightness;
    ESP_LOGI(CONFIG_TAG, "LED brightness set to %d percent", (int)(MINICO2CONFIG.led_cfg.brightness * 100));
    ESP_ERROR_CHECK(esp_event_post(CONFIG_EVENTS, LED_BRIGHTNESS_EVENT, NULL, 0, portMAX_DELAY));
}

/* Set the CO2 LED limits in PPM. All limits must be set at the same time. Each limit must be greater than the previous. */
void set_led_co2_limits(uint16_t medium_limit, uint16_t high_limit, uint16_t critical_limit){
    if (high_limit > critical_limit){
        high_limit = critical_limit;
        ESP_LOGE(CONFIG_TAG, "CO2 high limit forced to critical limit %d", critical_limit);
    }
    if (medium_limit > high_limit){
        medium_limit = high_limit;
        ESP_LOGE(CONFIG_TAG, "CO2 medium limit forced to high limit %d", high_limit);
    }
    MINICO2CONFIG.led_cfg.limit_medium = medium_limit;
    MINICO2CONFIG.led_cfg.limit_high = high_limit;
    MINICO2CONFIG.led_cfg.limit_critical = critical_limit;
    ESP_LOGI(CONFIG_TAG, "LED CO2 limit set to MEDIUM: %d PPM, HIGH: %d PPM, CRITICAL: %d PPM", 
    MINICO2CONFIG.led_cfg.limit_medium, MINICO2CONFIG.led_cfg.limit_high, MINICO2CONFIG.led_cfg.limit_critical);
    ESP_ERROR_CHECK(esp_event_post(CONFIG_EVENTS, CO2_LIMITS_EVENT, NULL, 0, portMAX_DELAY));
}

// Places the string representation of a minico2_cfg_s configuration struct into the buffer 'str'.
void config_to_str(char *str, size_t len, struct minico2_cfg_s *config)
{
    snprintf(str, len,
    "Nickname                 : '%s'\n"
    "Measurement period       : %d seconds\n"
    "Print measurements       : %s\n"
    "BLE                      : %s\n"
    "Zigbee                   : %s\n"
    "LED - brightness         : %d percent\n"
    "LED - Medium CO2 limit   : %d PPM\n"
    "LED - High CO2 limit     : %d PPM\n"
    "LED - Critical CO2 limit : %d PPM", 
    config->name, 
    config->measurement_period, 
    config->serial_print_enabled ? "ENABLED" : "DISABLED",
    config->ble_enabled ? "ENABLED" : "DISABLED",
    config->zigbee_enabled ? "ENABLED" : "DISABLED",
    (int)(config->led_cfg.brightness*100),
    config->led_cfg.limit_medium,
    config->led_cfg.limit_high,
    config->led_cfg.limit_critical
    );
}

// Logs 'config' to the serial port
void log_config(struct minico2_cfg_s *config)
{
    char config_str [1024] = "";
    config_to_str(config_str, sizeof(config_str), config);
    ESP_LOGI(CONFIG_TAG, "\n%s", config_str);
}

// Resets the configuration to default values
void reset_config(){
    set_print_sensor_readings(MINICO2CONFIG_DEFAULT.serial_print_enabled);
    set_nickname(&MINICO2CONFIG_DEFAULT.name);
    set_measurement_period(MINICO2CONFIG_DEFAULT.measurement_period);
    struct led_cfg_s led_cfg = MINICO2CONFIG_DEFAULT.led_cfg;
    set_led_brightness(led_cfg.brightness);
    set_led_co2_limits(led_cfg.limit_medium, led_cfg.limit_high, led_cfg.limit_critical);
}