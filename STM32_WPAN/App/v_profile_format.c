
#include "v_profile_format.h"
#include <stddef.h>

static uint16_t VSP_Format_ReadU16(const uint8_t *data)
{
    return (uint16_t)data[0] |
           ((uint16_t)data[1] << 8);
}

static uint32_t VSP_Format_ReadU32(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

int VSP_Profile_Validate(
    const uint8_t *data,
    uint32_t length)
{
    if ((data == NULL) ||
        (length < VSP_FORMAT_HEADER_SIZE))
    {
        return -1;
    }

    uint8_t version = data[0];
    uint8_t encoding = data[1];

    if ((version != VSP_FORMAT_VERSION) ||
        (encoding != VSP_FORMAT_ENCODING))
    {
        return -2;
    }

    if ((data[2] != 0U) || (data[3] != 0U))
    {
        return -2;
    }

    uint32_t sampleCount = VSP_Format_ReadU32(&data[4]);

    if (sampleCount == 0U)
    {
        return -3;
    }

    if (sampleCount >
        ((length - VSP_FORMAT_HEADER_SIZE) /
         VSP_FORMAT_RECORD_SIZE))
    {
        return -3;
    }

    uint32_t expectedLength =
        VSP_FORMAT_HEADER_SIZE +
        sampleCount * VSP_FORMAT_RECORD_SIZE;

    if (expectedLength != length)
    {
        return -3;
    }

    uint32_t previousTimestamp = 0U;

    for (uint32_t i = 0U; i < sampleCount; i++)
    {
        uint32_t offset =
            VSP_FORMAT_HEADER_SIZE +
            i * VSP_FORMAT_RECORD_SIZE;

        uint32_t timestamp =
            VSP_Format_ReadU32(&data[offset]);

        uint16_t voltage =
            VSP_Format_ReadU16(&data[offset + 4U]);

        if (voltage > VSP_MAX_VOLTAGE_MV)
        {
            return -4;
        }

        if (i == 0U)
        {
            if (timestamp != 0U)
            {
                return -5;
            }
        }
        else if (timestamp <= previousTimestamp)
        {
            return -5;
        }

        previousTimestamp = timestamp;
    }

    return 0;
}
