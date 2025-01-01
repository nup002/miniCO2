#include <stdbool.h>
#include "config.h"
#include "esp_log.h"
#include "../globals.h"

static const char *CONFIG_TAG = "config";

/* Enable or disable the printing of sensor measurements to the USB port */
void set_print_sensor_readings(bool enabled){
    MINICO2CONFIG.serial_print_enabled = enabled;
    if (MINICO2CONFIG.serial_print_enabled){
        ESP_LOGI(CONFIG_TAG, "Printing of measurements to the serial port ENABLED");
    }else{
        ESP_LOGI(CONFIG_TAG, "Printing of measurements to the serial port DISABLED");
    }
};

/* Set the nickname of the MiniCO2 */
void set_nickname(const char *nickname){
    if (sizeof(nickname) > 128){
        ESP_LOGE(CONFIG_TAG, "Nickname must be shorter than 128 characters.");
        return;
    }
    MINICO2CONFIG.name = nickname;
    ESP_LOGI(CONFIG_TAG, "Nickname set to '%s'", MINICO2CONFIG.name);
};

/* Set the measurement period in seconds. The value is clamped to the range (5 seconds, 1 year). */
void set_measurement_period(int period){
    if (period < 5){period = 5;}
    if (period > 60*60*24*365){period = 60*60*24*365;}
    MINICO2CONFIG.measurement_period = period;
    int p = MINICO2CONFIG.measurement_period;
    ESP_LOGI(CONFIG_TAG, "Measurement period set to %d hours, %d minutes, %d seconds.", p / 3600, (p % 3600) / 60, p % 60);
};

/* Set the LED brightness, from 0 (off) to 1 (brightest). The value is coerced to this range. */
void set_led_brightness(float brightness){
    if (brightness < 0.0){brightness = 0;}
    if (brightness > 1.0){brightness = 1;}
    MINICO2CONFIG.led_cfg.brightness = brightness;
    ESP_LOGI(CONFIG_TAG, "LED brightness set to %d percent", (int)(MINICO2CONFIG.led_cfg.brightness * 100));
};



