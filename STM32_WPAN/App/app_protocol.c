
#include "app_protocol.h"
#include <stddef.h>

/*
 * Read unsigned multibyte values from the wire.
 *
 * We use explicit byte operations rather than casting
 * received data into a C structure.
 *
 * Temporary wire encoding: little-endian.
 * This must be finalized before protocol interoperability.
 */
static uint16_t VSP_ReadU16(const uint8_t *data)
{
    return (uint16_t)data[0]
         | ((uint16_t)data[1] << 8);
}

static uint32_t VSP_ReadU32(const uint8_t *data)
{
    return (uint32_t)data[0]
         | ((uint32_t)data[1] << 8)
         | ((uint32_t)data[2] << 16)
         | ((uint32_t)data[3] << 24);
}

/*
 * Parse and validate a VSP packet header.
 *
 * Returns:
 *   0  = Valid packet
 *  -1  = Invalid arguments
 *  -2  = Packet too short
 *  -3  = Invalid header length
 *  -4  = Payload length mismatch
 *  -5  = Unsupported protocol version
 */
int VSP_ParsePacket(
    const uint8_t *data,
    uint16_t length,
    VSP_Header_t *header)
{
    if ((data == NULL) || (header == NULL))
    {
        return -1;
    }

    if (length < VSP_HEADER_SIZE)
    {
        return -2;
    }

    header->version       = data[0];
    header->command       = data[1];
    header->flags         = data[2];
    header->headerLength  = data[3];

    header->transferId    = VSP_ReadU32(&data[4]);
    header->sequence      = VSP_ReadU16(&data[8]);
    header->payloadLength = VSP_ReadU16(&data[10]);

    if (header->headerLength != VSP_HEADER_SIZE)
    {
        return -3;
    }

    if ((uint32_t)header->headerLength + header->payloadLength != length)
    {
        return -4;
    }

    if (header->version != VSP_VERSION_MAJOR)
    {
        return -5;
    }

    return 0;
}


/*
 * Write unsigned integers in little-endian wire order.
 */
static void VSP_WriteU16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)(value & 0xFFU);
    data[1] = (uint8_t)((value >> 8) & 0xFFU);
}

static void VSP_WriteU32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)(value & 0xFFU);
    data[1] = (uint8_t)((value >> 8) & 0xFFU);
    data[2] = (uint8_t)((value >> 16) & 0xFFU);
    data[3] = (uint8_t)((value >> 24) & 0xFFU);
}

/*
 * Encode a complete VSP packet.
 *
 * The header and payload are serialized into output.
 * No BLE functions are called here.
 */
int VSP_EncodePacket(
    const VSP_Header_t *header,
    const uint8_t *payload,
    uint8_t *output,
    uint16_t outputCapacity,
    uint16_t *outputLength)
{
    if ((header == NULL) ||
        (output == NULL) ||
        (outputLength == NULL))
    {
        return -1;
    }

    *outputLength = 0U;

    if ((header->payloadLength > 0U) && (payload == NULL))
    {
        return -1;
    }

    if (header->version != VSP_VERSION_MAJOR)
    {
        return -4;
    }

    if (header->headerLength != VSP_HEADER_SIZE)
    {
        return -1;
    }

    uint32_t totalLength =
        (uint32_t)VSP_HEADER_SIZE + header->payloadLength;

    if (totalLength > VSP_MAX_PACKET_SIZE)
    {
        return -3;
    }

    if (totalLength > outputCapacity)
    {
        return -2;
    }

    /* Fixed 12-byte header */
    output[0] = header->version;
    output[1] = header->command;
    output[2] = header->flags;
    output[3] = header->headerLength;

    VSP_WriteU32(&output[4], header->transferId);
    VSP_WriteU16(&output[8], header->sequence);
    VSP_WriteU16(&output[10], header->payloadLength);

    /* Optional payload */
    for (uint16_t i = 0U; i < header->payloadLength; i++)
    {
        output[VSP_HEADER_SIZE + i] = payload[i];
    }

    *outputLength = (uint16_t)totalLength;

    return 0;
}

