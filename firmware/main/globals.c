#include <stdbool.h>
#include "globals.h"
#include "types.h"

struct minico2_cfg_s MINICO2CONFIG = {0};  // Loaded from NVS during boot

struct minico2_cfg_s MINICO2CONFIG_DEFAULT = {
    .name = "MiniCO2",
    .measurement_period = 30,
    .serial_print_enabled = false,
    .ble_enabled = true,
    .zigbee_enabled = true,
    .led_cfg.brightness = 0.3,
    .led_cfg.limit_medium = 1000,
    .led_cfg.limit_high = 1500,
    .led_cfg.limit_critical = 2000
};