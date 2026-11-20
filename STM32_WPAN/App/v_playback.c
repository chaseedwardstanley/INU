
#include "v_playback.h"
#include "v_profile_format.h"
#include <stddef.h>

/* Read a little-endian 16-bit value. */
static uint16_t VSP_Playback_ReadU16(const uint8_t *data)
{
    return (uint16_t)data[0] |
           ((uint16_t)data[1] << 8);
}

/* Read a little-endian 32-bit value. */
static uint32_t VSP_Playback_ReadU32(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

/*
 * Convert 0–5000 mV to a 12-bit DAC code.
 * Rounded integer arithmetic.
 */
static uint16_t VSP_Playback_MvToDac(uint16_t millivolts)
{
    return (uint16_t)(
        ((uint32_t)millivolts * 4095U + 2500U) / 5000U
    );
}

void VSP_Playback_Init(VSP_Playback_t *playback)
{
    if (playback == NULL)
    {
        return;
    }

    playback->state = VSP_PLAYBACK_STOPPED;
    playback->profile = NULL;
    playback->sampleCount = 0U;
    playback->nextSample = 0U;
    playback->startTimeUs = 0U;
}

int VSP_Playback_Start(
    VSP_Playback_t *playback,
    const VSP_Profile_t *profile,
    uint32_t startTimeUs)
{
    if ((playback == NULL) || (profile == NULL))
    {
        return -1;
    }

    if (profile->state != VSP_PROFILE_READY)
    {
        return -2;
    }

    /* Validate the stored profile before playback. */
    if (VSP_Profile_Validate(
            profile->data,
            profile->expectedLength) != 0)
    {
        return -3;
    }

    playback->profile = profile;

    /* Sample count occupies bytes 4–7 of the header. */
    playback->sampleCount =
        VSP_Playback_ReadU32(&profile->data[4]);

    playback->nextSample = 0U;
    playback->startTimeUs = startTimeUs;
    playback->state = VSP_PLAYBACK_RUNNING;

    return 0;
}

void VSP_Playback_Stop(VSP_Playback_t *playback)
{
    if (playback == NULL)
    {
        return;
    }

    playback->state = VSP_PLAYBACK_STOPPED;
    playback->profile = NULL;
    playback->sampleCount = 0U;
    playback->nextSample = 0U;
    playback->startTimeUs = 0U;
}

int VSP_Playback_Update(
    VSP_Playback_t *playback,
    uint32_t nowUs,
    uint16_t *dacCode)
{
    if ((playback == NULL) || (dacCode == NULL))
    {
        return -1;
    }

    if (playback->state != VSP_PLAYBACK_RUNNING)
    {
        return 0;
    }

    const VSP_Profile_t *profile = playback->profile;

    if (profile == NULL)
    {
        playback->state = VSP_PLAYBACK_STOPPED;
        return -1;
    }

    uint32_t elapsedUs =
        nowUs - playback->startTimeUs;

    /*
     * Process all samples currently due.
     * If multiple samples are overdue, output the latest.
     */
    int updated = 0;

    while (playback->nextSample < playback->sampleCount)
    {
        uint32_t offset =
            VSP_FORMAT_HEADER_SIZE +
            playback->nextSample * VSP_FORMAT_RECORD_SIZE;

        const uint8_t *record =
            &profile->data[offset];

        uint32_t timestampUs =
            VSP_Playback_ReadU32(record);

        if (timestampUs > elapsedUs)
        {
            break;
        }

        uint16_t voltageMv =
            VSP_Playback_ReadU16(&record[4]);

        *dacCode = VSP_Playback_MvToDac(voltageMv);

        playback->nextSample++;
        updated = 1;
    }

    if (playback->nextSample >= playback->sampleCount)
    {
        playback->state = VSP_PLAYBACK_FINISHED;
    }

    return updated;
}
