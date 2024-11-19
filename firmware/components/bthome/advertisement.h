// Original source code from the bthome-weather-station project

#ifndef _BTHOME_ADVERTISEMENT_H_
#define _BTHOME_ADVERTISEMENT_H_

#include "constants.h"
#include "mbedtls/ccm.h"
#include "measurement.h"

#include <cstdint>
#include <string>

namespace bthome
{

    class Advertisement
    {
      public:
        Advertisement(void);
        Advertisement(std::string const& name);
        Advertisement(std::string const& name, bool encrypt, uint8_t const* const key);
        ~Advertisement();

        bool addMeasurement(Measurement const& measurement);

        const uint8_t* getPayload(void);
        uint32_t getPayloadSize(void) const;
        void reset(void);

      private:
        void writeHeader(void);
        void writeUuid(void);
        void writeDeviceInfo(void);
        void writeByte(uint8_t const data);
        void writeCounter(void);
        void writeMIC(uint8_t* mic);
        void doInit(std::string const& name, bool encrypt);
        void buildNonce(uint8_t* buf, uint32_t countId);

        bool m_encryptEnable;
        uint32_t m_encryptCount;
        mbedtls_ccm_context m_encryptCTX;
        uint8_t bindKey[constants::BIND_KEY_LEN];

        uint8_t m_deviceInfo;
        // Where in the data buffer does the next data byte go
        uint8_t m_dataIdx;

        // Index where data writing start, following header, UUID, device
        uint8_t m_sensorDataIdx;

        // The actual data
        uint8_t m_data[constants::BLE_ADVERT_MAX_LEN];
        // Where in the data is the service data size located
        // The size must be updated with every new sensor data inserted
        uint8_t m_serviceDataSizeIdx;
        // The UUID for the service data (unencrypted or not)
        uint16_t m_serviceUuid;
    };

    class AdvertisementWithId : public Advertisement
    {
      public:
        AdvertisementWithId(uint8_t const packetId);
        AdvertisementWithId(std::string const& name, uint8_t const packetId);
    };

}; // namespace bthome

#endif
