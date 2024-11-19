// Guard against double call to this file
#ifndef _SCD40_H
#define _SCD40_H

#include "scd4x.h"

#define SCD40_SDA GPIO_NUM_18
#define SCD40_SCL GPIO_NUM_20

extern i2c_dev_t SCD40DEV;

esp_err_t init_scd40(void);

void scd40_task(void *pvParameters);

#endif