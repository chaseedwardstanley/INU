
#ifndef VSP_PLAYBACK_H
#define VSP_PLAYBACK_H

#include <stdint.h>
#include "vsp_profile.h"

typedef enum
{
    VSP_PLAYBACK_STOPPED = 0,
    VSP_PLAYBACK_RUNNING,
    VSP_PLAYBACK_FINISHED
} VSP_PlaybackState_t;

typedef struct
{
    VSP_PlaybackState_t state;

    const VSP_Profile_t *profile;

    uint32_t sampleCount;
    uint32_t nextSample;

    uint32_t startTimeUs;

} VSP_Playback_t;

/*
 * Initialize the playback engine.
 */
void VSP_Playback_Init(VSP_Playback_t *playback);

/*
 * Start playback of a validated profile.
 *
 * startTimeUs is the current timestamp from a
 * monotonically increasing microsecond clock.
 *
 * Returns 0 on success.
 */
int VSP_Playback_Start(
    VSP_Playback_t *playback,
    const VSP_Profile_t *profile,
    uint32_t startTimeUs
);

/*
 * Stop playback.
 */
void VSP_Playback_Stop(VSP_Playback_t *playback);

/*
 * Determine whether a sample is due.
 *
 * Returns:
 *   1 = A DAC code is available
 *   0 = No sample due
 *  -1 = Invalid arguments
 */
int VSP_Playback_Update(
    VSP_Playback_t *playback,
    uint32_t nowUs,
    uint16_t *dacCode
);

#endif
