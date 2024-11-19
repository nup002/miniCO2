#ifndef _TYPES_H
#define _TYPES_H

#include <stdio.h>

#define str(x) #x
#define xstr(x) str(x)  // Permits printing of enums as strings

// Struct that holds one measurement made by the SCD40 sensor
struct SCD40measurement{
    uint16_t co2;
    float temperature;
    float humidity;
};

// The uniquely identified states that the LED can be in
enum LED_STATES {
  LOW_CO2,
  MEDIUM_CO2,
  HIGH_CO2,
  BOOTING_L,
  WIFI_CONNECTING_L,
  BT_CONNECTING_L,
  ERROR_L
};

// The various ways that the LED can appear to the user
enum LED_MODES {
  STATIC,
  FLASHING,
  PULSING
};

// The uniquely identified states that the device can be in
enum DEVICE_STATES {
  MEASURING,
  BOOTING,
  WIFI_CONNECTING,
  BT_CONNECTING,
  ERROR
};

// RGBA color struct
struct RGBA {
    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint16_t a;
};

// A struct that defines the visual state of the LED 
struct LED_VISUAL_STATE {
  struct RGBA clr;
  enum LED_MODES mode;
  float period; // Flash/pulse period in seconds
};

#endif