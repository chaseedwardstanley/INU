
#ifndef VSP_PROFILE_FORMAT_H
#define VSP_PROFILE_FORMAT_H

#include <stdint.h>

#define VSP_FORMAT_VERSION       1U
#define VSP_FORMAT_ENCODING     1U

#define VSP_FORMAT_HEADER_SIZE  8U
#define VSP_FORMAT_RECORD_SIZE  6U

#define VSP_MAX_VOLTAGE_MV      5000U

/*
 * Validate a complete voltage profile.
 *
 * Returns:
 *   0  = Valid profile
 *  -1  = Invalid arguments or truncated header
 *  -2  = Unsupported version or encoding
 *  -3  = Invalid sample count or total length
 *  -4  = Invalid voltage
 *  -5  = Non-increasing timestamps
 */
int VSP_Profile_Validate(
    const uint8_t *data,
    uint32_t length
);

#endif
