#ifndef _TYPES_H
#define _TYPES_H

#include <stdio.h>

struct SCD40measurement{
    uint16_t co2;
    float temperature;
    float humidity;
};

#endif