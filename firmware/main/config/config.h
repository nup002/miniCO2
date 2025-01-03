#ifndef _CONFIG_H
#define _CONFIG_H

#include <esp_event.h>
#include <stdbool.h>
#include "../types.h"

void set_print_sensor_readings(bool enabled);
void set_nickname(char *nickname);
void set_measurement_period(int period);
void set_led_brightness(float brightness);
void set_led_co2_limits(uint16_t medium_limit, uint16_t high_limit, uint16_t critical_limit);

void config_to_str(char *str, size_t len, struct minico2_cfg_s *config);
void log_config(struct minico2_cfg_s *config);
void reset_config();

ESP_EVENT_DECLARE_BASE(CONFIG_EVENTS);  // Declaration of the config events family

enum {                                  // Declaration of the specific config events
    PRINT_SENSOR_READINGS_EVENT,        // Print sensor readings setting changed
    NICKNAME_EVENT,                     // Sensor nickname changed
    MEASUREMENT_PERIOD_EVENT,           // Measurement period changed
    LED_BRIGHTNESS_EVENT,               // LED brightness changed
    CO2_LIMITS_EVENT                    // CO2 LED limits changed
};

#endif