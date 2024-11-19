// Original source code from the bthome-weather-station project

#ifndef BT_HOME_CONSTANTS_H_
#define BT_HOME_CONSTANTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>

namespace bthome
{

    namespace constants
    {

        constexpr uint16_t SERVICE_UUID {0xFCD2};
        constexpr uint8_t BLE_ADVERT_MAX_LEN {31};
        constexpr uint8_t MEASUREMENT_MAX_LEN {8};
        constexpr uint8_t NONCE_LEN {13};
        constexpr uint8_t COUNTER_LEN {4};
        constexpr uint8_t MIC_LEN {4};
        constexpr uint8_t BIND_KEY_LEN {16};
        constexpr uint8_t BTHOME_V2 {2};

        enum ObjectId
        {
            PACKET_ID             = 0x00,
            BATTERY               = 0x01,
            TEMPERATURE_PRECISE   = 0x02,
            HUMIDITY_PRECISE      = 0x03,
            PRESSURE              = 0x04,
            ILLUMINANCE           = 0x05,
            MASS_KILOS            = 0x06,
            MASS_POUNDS           = 0x07,
            DEW_POINT             = 0x08,
            COUNT_SMALL           = 0x09,
            ENERGY                = 0x0A,
            POWER                 = 0x0B,
            VOLTAGE               = 0x0C,
            PM_2_5                = 0x0D,
            PM_10                 = 0x0E,
            GENERIC_BOOL          = 0x0F,
            POWER_STATE           = 0x10,
            OPENING_STATE         = 0x11,
            CO2                   = 0x12,
            TOTAL_VOC             = 0x13,
            MOISTURE_PRECISE      = 0x14,
            BATTERY_STATE         = 0x15,
            BATTERY_CHARGE_STATE  = 0x16,
            CARBON_MONOXIDE_STATE = 0x17,
            COLD_STATE            = 0x18,
            CONNECTIVITY_STATE    = 0x19,
            DOOR_STATE            = 0x1A,
            GARAGE_DOOR_STATE     = 0x1B,
            GAS_STATE             = 0x1C,
            HEAT_STATE            = 0x1D,
            LIGHT_STATE           = 0x1E,
            LOCK_STATE            = 0x1F,
            MOISTURE_STATE        = 0x20,
            MOTION_STATE          = 0x21,
            MOVING_STATE          = 0x22,
            OCCUPANCY_STATE       = 0x23,
            PLUG_STATE            = 0x24,
            PRESENCE_STATE        = 0x25,
            PROBLEM_STATE         = 0x26,
            RUNNING_STATE         = 0x27,
            SAFETY_STATE          = 0x28,
            SMOKE_STATE           = 0x29,
            SOUND_STATE           = 0x2A,
            TAMPER_STATE          = 0x2B,
            VIBRATION_STATE       = 0x2C,
            WINDOW_STATE          = 0x2D,
            HUMIDITY_COARSE       = 0x2E,
            MOISTURE_COARSE       = 0x2F,
            BUTTON_EVENT          = 0x3A,
            DIMMER_EVENT          = 0x3C,
            COUNT_MEDIUM          = 0x3D,
            COUNT_LARGE           = 0x3D,
            ROTATION              = 0x3F,
            DISTANCE_MILLIMETERS  = 0x40,
            DISTANCE_METERS       = 0x41,
            DURATION              = 0x42,
            CURRENT               = 0x43,
            SPEED                 = 0x44,
            TEMPERATURE_COARSE    = 0x45,
            UV_INDEX              = 0x46,
            LAST_DEFINED_ID       = 0x47,
            MAX_OBJECT_ID         = 256
        };

        typedef struct
        {
            // The scaling factor for encoding
            uint16_t factor;
            // The length in bytes
            uint8_t length;
        } BTHomeDataTypeInfo;

        extern const BTHomeDataTypeInfo InfoLookup[ObjectId::MAX_OBJECT_ID];

        enum BTHOME_DEVICE_INFO_SHIFTS
        {
            ENCRYPTED = 0,
            VERSION   = 5
        };

        enum BLE_ADVERT_DATA_TYPE
        {
            TYPE          = 0x1,
            SHORT_NAME    = 0x8,
            COMPLETE_NAME = 0x9,
            SERVICE_DATA  = 0x16
        };

        enum BLE_FLAGS_DATA_TYPE
        {
            GENERAL_DISCOVERY   = (0x01 << 1),
            BREDR_NOT_SUPPORTED = (0x01 << 2),
        };

    }; // namespace constants

}; // namespace bthome

#ifdef __cplusplus
}
#endif

#endif
