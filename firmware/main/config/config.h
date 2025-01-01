#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdbool.h>

void set_print_sensor_readings(bool enabled);
void set_nickname(const char *nickname);
void set_measurement_period(int period);
void set_led_brightness(float brightness);

#endif