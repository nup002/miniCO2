// Original source code from the bthome-weather-station project

#ifdef __cplusplus
extern "C" {
#endif

#include "measurement.h"

#include "esp_log.h"

#include <cassert>
#include <cstring>

#define NAME "bthome"

namespace bthome
{

    Measurement::Measurement(enum constants::ObjectId const objectId, float const data)
        : m_objectId(objectId), m_scaledData(0), m_payloadIdx {0}
    {
        assert(objectId < constants::LAST_DEFINED_ID);
        this->m_objectId                                   = objectId;
        constants::BTHomeDataTypeInfo const* const infoPtr = &constants::InfoLookup[objectId];
        this->m_scaledData                                 = static_cast<uint64_t>(data * infoPtr->factor);
        this->packData();
    }
    Measurement::Measurement(enum constants::ObjectId const objectId, uint64_t const data)
        : m_objectId(objectId), m_scaledData(data), m_payloadIdx {0}
    {
        assert(objectId < constants::LAST_DEFINED_ID);
        this->packData();
    }

    Measurement::~Measurement(void) { }

    void Measurement::packData(void)
    {
        constants::BTHomeDataTypeInfo const* const infoPtr = &constants::InfoLookup[this->m_objectId];

        this->m_payload[this->m_payloadIdx] = static_cast<uint8_t>(this->m_objectId);
        this->m_payloadIdx++;

        for (int8_t size = 0; size < infoPtr->length; size++)
        {
            this->m_payload[this->m_payloadIdx] = static_cast<uint8_t>((this->m_scaledData >> (8 * size)) & 0xff);
            this->m_payloadIdx++;
        }
    }

    const uint8_t* Measurement::getPayload(void) const
    {
        return &this->m_payload[0];
    }

    uint32_t Measurement::getPayloadSize(void) const
    {
        return this->m_payloadIdx;
    }

}; // namespace bthome

#ifdef __cplusplus
}
#endif
