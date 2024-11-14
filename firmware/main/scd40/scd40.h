// Guard against double call to this file
#ifndef _SCD40_H
#define _SCD40_H

#include "scd4x.h"

extern i2c_dev_t SCD40DEV;

static const int SCD40_SDA = 18;
static const int SCD40_SCL = 20;

struct SCD40measurement{
    uint16_t co2;
    float temperature;
    float humidity;
};

esp_err_t init_scd40(void);

void scd40_task(void *pvParameters);

#endif