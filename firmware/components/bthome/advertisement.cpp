// Original source code from the bthome-weather-station project

#include "advertisement.h"

#include "constants.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_random.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/types.h>

#define _ADVERT_LOG_NAME "advert"

namespace bthome
{
    void Advertisement::doInit(std::string const& name, bool encrypt)
    {

        m_dataIdx            = 0;
        m_serviceDataSizeIdx = 0;
        m_sensorDataIdx      = 0;
        m_serviceUuid        = constants::SERVICE_UUID;
        // Write the BLE flags
        this->writeHeader();

        this->m_encryptEnable = encrypt;

        if (!name.empty())
        {

            // Track where the length will go
            uint8_t startIdx = this->m_dataIdx;
            // Write the length as 0 for now
            this->writeByte(0);

            // This is a complete name
            this->writeByte(constants::BLE_ADVERT_DATA_TYPE::COMPLETE_NAME);

            // Write the actual bytes now
            for (uint8_t idx = 0; idx < name.size(); idx++)
            {
                this->writeByte(name[idx]);
            }

            // Write the length now
            this->m_data[startIdx] = this->m_dataIdx - startIdx - 1;
        }
        // Follow up the name with the service data UUID (but no data yet)
        this->writeUuid();

        // Write BTHome device info
        this->writeDeviceInfo();

        // Actual data goes here
        this->m_sensorDataIdx = this->m_dataIdx;
    }

    Advertisement::Advertisement(void)
    {
        doInit(std::string(""), false);
    }

    Advertisement::Advertisement(std::string const& name)
    {
        doInit(name, false);
    }

    Advertisement::Advertisement(std::string const& name, bool encrypt, uint8_t const* const key)
    {
        memcpy(bindKey, key, sizeof(uint8_t) * constants::BIND_KEY_LEN);
        m_encryptCount = esp_random() % 0x427;
        mbedtls_ccm_init(&this->m_encryptCTX);
        mbedtls_ccm_setkey(&m_encryptCTX, MBEDTLS_CIPHER_ID_AES, bindKey, constants::BIND_KEY_LEN * 8);
        doInit(name, encrypt);
    }
    Advertisement::~Advertisement(void) { }

    void Advertisement::reset(void)
    {
        this->m_dataIdx = 0;
    }

    void Advertisement::writeHeader(void)
    {
        // Length for this is fixed
        this->writeByte(2);

        this->writeByte(constants::BLE_ADVERT_DATA_TYPE::TYPE);

        this->writeByte(constants::BLE_FLAGS_DATA_TYPE::BREDR_NOT_SUPPORTED |
                        constants::BLE_FLAGS_DATA_TYPE::GENERAL_DISCOVERY);
    }

    void Advertisement::writeCounter(void)
    {
        uint32_t* counter_p = (uint32_t*)&m_data[this->m_dataIdx];
        *counter_p          = m_encryptCount;
        m_encryptCount++;
        this->m_dataIdx += constants::COUNTER_LEN;
        this->m_data[this->m_serviceDataSizeIdx] += constants::COUNTER_LEN;
    }

    void Advertisement::writeMIC(uint8_t* mic)
    {
        memcpy(&m_data[this->m_dataIdx], mic, constants::MIC_LEN);
        this->m_dataIdx += constants::MIC_LEN;
        this->m_data[this->m_serviceDataSizeIdx] += constants::MIC_LEN;
    }

    void Advertisement::writeUuid(void)
    {
        this->m_serviceDataSizeIdx = this->m_dataIdx;
        // Service data + UUID length is fixed
        this->writeByte(3);

        this->writeByte(constants::BLE_ADVERT_DATA_TYPE::SERVICE_DATA);

        this->writeByte(this->m_serviceUuid & 0xff);
        this->writeByte((this->m_serviceUuid >> 8) & 0xff);
    }

    void Advertisement::writeDeviceInfo(void)
    {
        this->m_deviceInfo = (this->m_encryptEnable << constants::BTHOME_DEVICE_INFO_SHIFTS::ENCRYPTED) |
                             (constants::BTHOME_V2 << constants::BTHOME_DEVICE_INFO_SHIFTS::VERSION);
        this->writeByte(this->m_deviceInfo);

        this->m_data[this->m_serviceDataSizeIdx] += 1;
    }

    inline void Advertisement::writeByte(uint8_t const data)
    {
        if (this->m_dataIdx <= constants::BLE_ADVERT_MAX_LEN)
        {
            this->m_data[this->m_dataIdx] = data;
            this->m_dataIdx++;
        }
        else
        {
            ESP_LOGE(_ADVERT_LOG_NAME, "Discarding data");
        }
    }

    bool Advertisement::addMeasurement(Measurement const& measurement)
    {

        if ((this->m_dataIdx + measurement.getPayloadSize()) > constants::BLE_ADVERT_MAX_LEN)
        {
            ESP_LOGE(_ADVERT_LOG_NAME, "Unable to add sensor");
            return false;
        }

        memcpy(&this->m_data[this->m_dataIdx], measurement.getPayload(), measurement.getPayloadSize());

        // Increment where the next measurement will go
        this->m_dataIdx += measurement.getPayloadSize();

        // Also increase the length in the header
        this->m_data[this->m_serviceDataSizeIdx] += measurement.getPayloadSize();

        return true;
    }

    void Advertisement::buildNonce(uint8_t* buf, uint32_t countId)
    {
        uint8_t macAddr[6] = {0};
        uint8_t* uuidPtr   = (uint8_t*)(&this->m_serviceUuid);
        uint8_t* countPtr  = (uint8_t*)(&countId);
        ESP_ERROR_CHECK(esp_read_mac(&buf[0], ESP_MAC_BT));

        memcpy(&buf[6], uuidPtr, 2);
        buf[8] = m_deviceInfo;
        memcpy(&buf[9], countPtr, 4);
    }

    const uint8_t* Advertisement::getPayload(void)
    {
        if (this->m_encryptEnable)
        {
            size_t textLen = this->m_dataIdx - this->m_sensorDataIdx;
            uint8_t ciphertext[constants::BLE_ADVERT_MAX_LEN];
            uint8_t encryptionTag[constants::BLE_ADVERT_MAX_LEN];
            uint8_t nonce[constants::NONCE_LEN];

            buildNonce(nonce, m_encryptCount);
            mbedtls_ccm_encrypt_and_tag(&m_encryptCTX, textLen, nonce, constants::NONCE_LEN, 0, 0,
                                        &this->m_data[m_sensorDataIdx], &ciphertext[0], encryptionTag,
                                        constants::MIC_LEN);
            // replace plaintext as ciphertext
            memcpy(&this->m_data[m_sensorDataIdx], ciphertext, textLen);

            writeCounter();
            writeMIC(encryptionTag);

            return &this->m_data[0];
        }
        else
        {
            return &this->m_data[0];
        }
    }

    uint32_t Advertisement::getPayloadSize(void) const
    {
        return this->m_dataIdx;
    }

    AdvertisementWithId::AdvertisementWithId(uint8_t const packetId) : Advertisement()
    {
        Measurement packetIdData(constants::ObjectId::PACKET_ID, static_cast<uint64_t>(packetId));
        this->addMeasurement(packetIdData);
    }

    AdvertisementWithId::AdvertisementWithId(std::string const& name, uint8_t const packetId) : Advertisement(name)
    {
        Measurement packetIdData(constants::ObjectId::PACKET_ID, static_cast<uint64_t>(packetId));
        this->addMeasurement(packetIdData);
    }

}; // namespace bthome
