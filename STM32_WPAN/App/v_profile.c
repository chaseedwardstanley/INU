
#include "v_profile.h"
#include "v_profile_format.h"
#include <stddef.h>

void VSP_Profile_Init(VSP_Profile_t *profile)
{
    if (profile == NULL)
    {
        return;
    }

    profile->state = VSP_PROFILE_EMPTY;

    profile->transferId = 0U;
    profile->expectedLength = 0U;
    profile->receivedLength = 0U;
    profile->nextSequence = 0U;

    /*
     * The data buffer is intentionally not cleared.
     *
     * Its contents are invalid until a complete
     * profile has been received and verified.
     */
}


int VSP_Profile_Begin(
    VSP_Profile_t *profile,
    uint32_t transferId,
    uint32_t expectedLength)
{
    if (profile == NULL)
    {
        return -1;
    }

    /*
     * Reject a new transfer while one is active.
     */
    if (profile->state == VSP_PROFILE_RECEIVING)
    {
        return -4;
    }

    /*
     * A profile must contain at least one byte.
     */
    if (expectedLength == 0U)
    {
        return -3;
    }

    /*
     * Reject profiles larger than our allocated buffer.
     */
    if (expectedLength > VSP_PROFILE_CAPACITY)
    {
        return -2;
    }

    /*
     * Accept the new transfer.
     */
    profile->transferId = transferId;
    profile->expectedLength = expectedLength;
    profile->receivedLength = 0U;
    profile->nextSequence = 0U;

    profile->state = VSP_PROFILE_RECEIVING;

    return 0;
}


int VSP_Profile_Write(
    VSP_Profile_t *profile,
    uint32_t transferId,
    uint16_t sequence,
    const uint8_t *data,
    uint16_t length)
{
    if ((profile == NULL) || (data == NULL) || (length == 0U))
    {
        return -1;
    }

    if (profile->state != VSP_PROFILE_RECEIVING)
    {
        return -2;
    }

    if (transferId != profile->transferId)
    {
        return -3;
    }

    if (sequence != profile->nextSequence)
    {
        return -4;
    }

    /*
     * Prevent a uint16_t sequence counter from wrapping.
     */
    if (profile->nextSequence == UINT16_MAX)
    {
        return -6;
    }

    /*
     * Ensure this chunk fits within the declared profile size.
     * Subtraction avoids overflow from receivedLength + length.
     */
    if ((profile->receivedLength > profile->expectedLength) ||
        ((uint32_t)length >
         (profile->expectedLength - profile->receivedLength)))
    {
        return -5;
    }

    /*
     * Copy the chunk into the profile buffer.
     */
    for (uint16_t i = 0U; i < length; i++)
    {
        profile->data[profile->receivedLength + i] = data[i];
    }

    profile->receivedLength += length;
    profile->nextSequence++;

    return 0;
}


/*
 * CRC-32/ISO-HDLC
 *
 * Polynomial: 0x04C11DB7
 * Reflected polynomial: 0xEDB88320
 * Initial value: 0xFFFFFFFF
 * Final XOR: 0xFFFFFFFF
 */
static uint32_t VSP_Profile_Crc32(
    const uint8_t *data,
    uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFU;

    for (uint32_t i = 0U; i < length; i++)
    {
        crc ^= (uint32_t)data[i];

        for (uint8_t bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 1U) != 0U)
            {
                crc = (crc >> 1) ^ 0xEDB88320U;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc ^ 0xFFFFFFFFU;
}


int VSP_Profile_Complete(
    VSP_Profile_t *profile,
    uint32_t transferId,
    uint32_t expectedCrc)
{
    if (profile == NULL)
    {
        return -1;
    }

    /*
     * A transfer must be active.
     */
    if (profile->state != VSP_PROFILE_RECEIVING)
    {
        return -2;
    }

    /*
     * The completion packet must belong
     * to the active transfer.
     */
    if (transferId != profile->transferId)
    {
        return -3;
    }

    /*
     * Every declared byte must have arrived.
     */
    if (profile->receivedLength != profile->expectedLength)
    {
        return -4;
    }

    /*
     * Calculate CRC over the complete profile data.
     * The VSP transport headers are excluded.
     */
    uint32_t calculatedCrc = VSP_Profile_Crc32(
        profile->data,
        profile->receivedLength
    );

    if (calculatedCrc != expectedCrc)
    {
        profile->state = VSP_PROFILE_ERROR;
        return -5;
    }

        /*
        * The profile is now verified and available
        * for playback.
        */
        /*
    * CRC confirms data integrity.
    * Format validation confirms the data is usable.
    */
    int validationResult = VSP_Profile_Validate(
        profile->data,
        profile->receivedLength
    );

    if (validationResult != 0)
    {
        profile->state = VSP_PROFILE_ERROR;
        return -6;
    }

    profile->state = VSP_PROFILE_READY;

    return 0;
}

int VSP_Profile_Abort(
    VSP_Profile_t *profile,
    uint32_t transferId)
{
    if (profile == NULL)
    {
        return -1;
    }

    if ((profile->state != VSP_PROFILE_RECEIVING) ||
        (profile->transferId != transferId))
    {
        return -2;
    }

    VSP_Profile_Init(profile);
    return 0;
}

