// Original source code from the bthome-weather-station project

#include "constants.h"

namespace bthome
{

    namespace constants
    {

        const BTHomeDataTypeInfo InfoLookup[ObjectId::MAX_OBJECT_ID] = {
            // PACKET_ID
            {.factor = 1, .length = 1},
            // BATTERY
            {.factor = 1, .length = 1},
            // TEMPERATURE_PRECISE
            {.factor = 100, .length = 2},
            // HUMIDITY_PRECISE
            {.factor = 100, .length = 2},
            // PRESSURE
            {.factor = 100, .length = 3},
            // ILLUMINANCE
            {.factor = 100, .length = 3},
            // MASS_KILOS
            {.factor = 100, .length = 2},
            // MASS_POUNDS
            {.factor = 100, .length = 2},
            // DEW_POINT
            {.factor = 100, .length = 2},
            // COUNT_SMALL
            {.factor = 1, .length = 1},
            // ENERGY
            {.factor = 100, .length = 3},
            // POWER
            {.factor = 100, .length = 3},
            // VOLTAGE
            {.factor = 1000, .length = 2},
            // PM_2_5
            {.factor = 1, .length = 2},
            // PM_10
            {.factor = 1, .length = 2},
            // GENERIC_BOOL
            {.factor = 1, .length = 1},
            // POWER_STATE
            {.factor = 1, .length = 1},
            // OPENING_STATE
            {.factor = 1, .length = 1},
            // CO2
            {.factor = 1, .length = 2},
            // TOTAL_VOC
            {.factor = 1, .length = 1},
            // MOISTURE_PRECISE
            {.factor = 100, .length = 2},
            // BATTERY_STATE
            {.factor = 1, .length = 1},
            // BATTERY_CHARGE_STATE
            {.factor = 1, .length = 1},
            // CARBON_MONOXIDE_STATE
            {.factor = 1, .length = 1},
            // COLD_STATE
            {.factor = 1, .length = 1},
            // CONNECTIVITY_STATE
            {.factor = 1, .length = 1},
            // DOOR_STATE
            {.factor = 1, .length = 1},
            // GARAGE_DOOR_STATE
            {.factor = 1, .length = 1},
            // GAS_STATE
            {.factor = 1, .length = 1},
            // HEAT_STATE
            {.factor = 1, .length = 1},
            // LIGHT_STATE
            {.factor = 1, .length = 1},
            // LOCK_STATE
            {.factor = 1, .length = 1},
            // MOISTURE_STATE
            {.factor = 1, .length = 1},
            // MOTION_STATE
            {.factor = 1, .length = 1},
            // MOVING_STATE
            {.factor = 1, .length = 1},
            // OCCUPANCY_STATE
            {.factor = 1, .length = 1},
            // PLUG_STATE
            {.factor = 1, .length = 1},
            // PRESENCE_STATE
            {.factor = 1, .length = 1},
            // PROBLEM_STATE
            {.factor = 1, .length = 1},
            // RUNNING_STATE
            {.factor = 1, .length = 1},
            // SAFETY_STATE
            {.factor = 1, .length = 1},
            // SMOKE_STATE
            {.factor = 1, .length = 1},
            // SOUND_STATE
            {.factor = 1, .length = 1},
            // TAMPER_STATE
            {.factor = 1, .length = 1},
            // VIBRATION_STATE
            {.factor = 1, .length = 1},
            // WINDOW_STATE
            {.factor = 1, .length = 1},
            // HUMIDITY_COARSE
            {.factor = 1, .length = 1},
            // MOISTURE_COARSE
            {.factor = 1, .length = 1},
            // BUTTON_EVENT
            // DIMMER_EVENT
            // COUNT_MEDIUM
            {.factor = 1, .length = 2},
            // COUNT_LARGE
            {.factor = 1, .length = 4},
            // ROTATION
            {.factor = 10, .length = 2},
            // DISTANCE_MILLIMETERS
            {.factor = 1, .length = 2},
            // DISTANCE_METERS
            {.factor = 10, .length = 2},
            // DURATION
            {.factor = 1000, .length = 3},
            // CURRENT
            {.factor = 1000, .length = 2},
            // SPEED
            {.factor = 100, .length = 2},
            // TEMPERATURE_COARSE
            {.factor = 10, .length = 2},
            // UV_INDEX
            {.factor = 10, .length = 1},
        };

    };
}; // namespace bthome
