
#ifndef VSP_PROFILE_H
#define VSP_PROFILE_H

#include <stdint.h>

/*
 * Profile transfer states.
 */
typedef enum
{
    VSP_PROFILE_EMPTY = 0,
    VSP_PROFILE_RECEIVING,
    VSP_PROFILE_READY,
    VSP_PROFILE_ERROR
} VSP_ProfileState_t;

/*
 * Profile storage.
 *
 * Initial capacity: 4096 bytes.
 * We will verify SRAM availability before increasing it.
 */
#define VSP_PROFILE_CAPACITY (48U * 1024U)

typedef struct
{
    VSP_ProfileState_t state;

    uint32_t transferId;
    uint32_t expectedLength;
    uint32_t receivedLength;

    uint16_t nextSequence;

    uint8_t data[VSP_PROFILE_CAPACITY];

} VSP_Profile_t;

/*
 * Initialize the profile manager.
 */
void VSP_Profile_Init(VSP_Profile_t *profile);

/*
 * Start receiving a new profile.
 *
 * Returns:
 *   0  = Transfer accepted
 *  -1  = Invalid arguments
 *  -2  = Profile exceeds buffer capacity
 *  -3  = Invalid profile length
 *  -4  = Transfer already in progress
 */
int VSP_Profile_Begin(
    VSP_Profile_t *profile,
    uint32_t transferId,
    uint32_t expectedLength
);

/*
 * Append a data chunk to the active profile transfer.
 *
 * Returns:
 *   0  = Chunk accepted
 *  -1  = Invalid arguments
 *  -2  = No transfer in progress
 *  -3  = Transfer ID mismatch
 *  -4  = Unexpected sequence number
 *  -5  = Chunk exceeds declared profile length
 *  -6  = Sequence number exhausted
 */
int VSP_Profile_Write(
    VSP_Profile_t *profile,
    uint32_t transferId,
    uint16_t sequence,
    const uint8_t *data,
    uint16_t length
);

/*
 * Verify and finalize a received profile.
 *
 * Returns:
 *   0  = Profile verified and ready
 *  -1  = Invalid arguments
 *  -2  = No transfer in progress
 *  -3  = Transfer ID mismatch
 *  -4  = Incomplete transfer
 *  -5  = CRC-32 mismatch
 */
int VSP_Profile_Complete(
    VSP_Profile_t *profile,
    uint32_t transferId,
    uint32_t expectedCrc
);

/*
 * Abort an active profile transfer.
 *
 * Returns:
 *   0  = Transfer aborted
 *  -1  = Invalid arguments
 *  -2  = No matching active transfer
 */
int VSP_Profile_Abort(
    VSP_Profile_t *profile,
    uint32_t transferId
);

#endif /* VSP_PROFILE_H */
