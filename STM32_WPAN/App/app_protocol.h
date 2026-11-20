
#ifndef VSP_PROTOCOL_H
#define VSP_PROTOCOL_H

#include <stdint.h>

/*
 * VoltageSync Protocol — VSP V1.01
 *
 * This file defines the protocol's public interface.
 * It does not depend on STM32 hardware or BLE libraries.
 */

/* Protocol version */
#define VSP_VERSION_MAJOR       1U
#define VSP_VERSION_MINOR       1U

/* Fixed packet header size */
#define VSP_HEADER_SIZE         12U

/* Maximum BLE characteristic value length */
#define VSP_MAX_PACKET_SIZE     153U

/* Maximum payload after the VSP header */
#define VSP_MAX_PAYLOAD_SIZE    (VSP_MAX_PACKET_SIZE - VSP_HEADER_SIZE)

/*
 * Message identifiers
 */
typedef enum
{
    VSP_CMD_HELLO             = 0x01,
    VSP_CMD_CONFIG            = 0x02,
    VSP_CMD_STATUS            = 0x03,

    VSP_CMD_EASTER_EGG        = 0x07,

    VSP_CMD_PROFILE_BEGIN     = 0x10,
    VSP_CMD_PROFILE_DATA      = 0x11,
    VSP_CMD_PROFILE_COMPLETE  = 0x12,
    VSP_CMD_PROFILE_READY     = 0x13,
    VSP_CMD_PROFILE_ABORT     = 0x14,

    VSP_CMD_PLAY              = 0x21,
    VSP_CMD_STOP              = 0x22

} VSP_Command_t;

/*
 * Parsed representation of a packet header.
 *
 * IMPORTANT:
 * This structure is for application use.
 * We will NOT transmit it directly as raw memory.
 */
typedef struct
{
    uint8_t  version;
    uint8_t  command;
    uint8_t  flags;
    uint8_t  headerLength;

    uint32_t transferId;
    uint16_t sequence;
    uint16_t payloadLength;

} VSP_Header_t;

int VSP_ParsePacket(
    const uint8_t *data,
    uint16_t length,
    VSP_Header_t *header
);

int VSP_EncodePacket(
    const VSP_Header_t *header,
    const uint8_t *payload,
    uint8_t *output,
    uint16_t outputCapacity,
    uint16_t *outputLength
);

#endif /* VSP_PROTOCOL_H */
