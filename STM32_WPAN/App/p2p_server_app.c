/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App/p2p_server_app.c
  * @author  MCD Application Team
  * @brief   Peer to peer Server Application
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "p2p_server_app.h"
#include "stm32_seq.h"
#include "tim.h"
#include "stm32wbxx_hal_tim.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_protocol.h"
#include "v_profile.h"
#include "v_playback.h"
#include "v_profile_format.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define VSP_CMD_GET_DIAGNOSTICS 0x30U
#define VSP_DIAGNOSTICS_PAYLOAD_SIZE 25U

/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern volatile uint32_t tim2CompareCount; /* Defined in main.c */
static VSP_Profile_t vspProfile;
static VSP_Playback_t vspPlayback;
/* Read-only diagnostic snapshot, updated by the sequencer playback task. */
static volatile uint32_t sampleTimeUs[2] = {0U, 0U};
static volatile uint32_t sampleIntervalUs = 0U;
static volatile uint16_t lastDacCode = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void VSP_SendResponse(
    const VSP_Header_t *request,
    uint8_t command,
    int status
);

static void VSP_Playback_Task(void);
static void VSP_SendDiagnostics(const VSP_Header_t *request);
static void VSP_WriteU16LE(uint8_t *dest, uint16_t value);
static void VSP_WriteU32LE(uint8_t *dest, uint32_t value);
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void P2PS_STM_App_Notification(P2PS_STM_App_Notification_evt_t *pNotification)
{
/* USER CODE BEGIN P2PS_STM_App_Notification_1 */

/* USER CODE END P2PS_STM_App_Notification_1 */
  switch(pNotification->P2P_Evt_Opcode)
  {
/* USER CODE BEGIN P2PS_STM_App_Notification_P2P_Evt_Opcode */

/* USER CODE END P2PS_STM_App_Notification_P2P_Evt_Opcode */

    case P2PS_STM__NOTIFY_ENABLED_EVT:
/* USER CODE BEGIN P2PS_STM__NOTIFY_ENABLED_EVT */

/* USER CODE END P2PS_STM__NOTIFY_ENABLED_EVT */
      break;

    case P2PS_STM_NOTIFY_DISABLED_EVT:
/* USER CODE BEGIN P2PS_STM_NOTIFY_DISABLED_EVT */

/* USER CODE END P2PS_STM_NOTIFY_DISABLED_EVT */
      break;

    case P2PS_STM_WRITE_EVT:
/* USER CODE BEGIN P2PS_STM_WRITE_EVT */
        {
            const uint8_t *packet =
                pNotification->DataTransfered.pPayload;

            uint16_t packetLength =
                pNotification->DataTransfered.Length;

            VSP_Header_t header;

            int result = VSP_ParsePacket(
                packet,
                packetLength,
                &header
            );

            if (result == 0)
            {
                const uint8_t *payload = &packet[VSP_HEADER_SIZE];

                if (packetLength > VSP_MAX_PACKET_SIZE)
                {
                    break;
                }

                switch (header.command)
                {
                    case VSP_CMD_HELLO:
                    {
                        VSP_SendResponse(
                            &header,
                            VSP_CMD_HELLO,
                            0
                        );
                        break;
                    }

                    case VSP_CMD_PROFILE_BEGIN:
                    {
                        if (header.payloadLength != 4U)
                        {
                            VSP_SendResponse(
                                &header,
                                VSP_CMD_PROFILE_BEGIN,
                                -1
                            );
                            break;
                        }

                        uint32_t expectedLength =
                            (uint32_t)payload[0] |
                            ((uint32_t)payload[1] << 8) |
                            ((uint32_t)payload[2] << 16) |
                            ((uint32_t)payload[3] << 24);

                        result = VSP_Profile_Begin(
                            &vspProfile,
                            header.transferId,
                            expectedLength
                        );

                        VSP_SendResponse(
                            &header,
                            VSP_CMD_PROFILE_BEGIN,
                            result
                        );
                        break;
                    }

                    case VSP_CMD_PROFILE_DATA:
                    {
                        result = VSP_Profile_Write(
                            &vspProfile,
                            header.transferId,
                            header.sequence,
                            payload,
                            header.payloadLength
                        );

                        VSP_SendResponse(
                            &header,
                            VSP_CMD_PROFILE_DATA,
                            result
                        );
                        break;
                    }

                    case VSP_CMD_PROFILE_COMPLETE:
                    {
                        if (header.payloadLength != 4U)
                        {
                            VSP_SendResponse(
                                &header,
                                VSP_CMD_PROFILE_COMPLETE,
                                -1
                            );
                            break;
                        }

                        uint32_t expectedCrc =
                            (uint32_t)payload[0] |
                            ((uint32_t)payload[1] << 8) |
                            ((uint32_t)payload[2] << 16) |
                            ((uint32_t)payload[3] << 24);

                        result = VSP_Profile_Complete(
                            &vspProfile,
                            header.transferId,
                            expectedCrc
                        );

                        VSP_SendResponse(
                            &header,
                            VSP_CMD_PROFILE_READY,
                            result
                        );
                        break;
                    }

                    case VSP_CMD_PROFILE_ABORT:
                    {
                        result = VSP_Profile_Abort(
                            &vspProfile,
                            header.transferId
                        );

                        VSP_SendResponse(
                            &header,
                            VSP_CMD_PROFILE_ABORT,
                            result
                        );
                        break;
                    }

                    case VSP_CMD_PLAY:
                    {
                        if (header.payloadLength != 0U)
                        {
                            VSP_SendResponse(
                                &header,
                                VSP_CMD_PLAY,
                                -1
                            );
                            break;
                        }

                        uint32_t startTimeUs = __HAL_TIM_GET_COUNTER(&htim2);

                        result = VSP_Playback_Start(
                            &vspPlayback,
                            &vspProfile,
                            startTimeUs
                        );

                        if (result == 0)
                        {
                            /* Start each successful playback with clean diagnostics. */
                            __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);
                            __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
                            sampleTimeUs[0] = 0U;
                            sampleTimeUs[1] = 0U;
                            sampleIntervalUs = 0U;
                            lastDacCode = 0U;
                            UTIL_SEQ_SetTask(
                                1UL << CFG_TASK_VSP_PLAYBACK_ID,
                                CFG_SCH_PRIO_0
                            );
                        }

                        VSP_SendResponse(
                            &header,
                            VSP_CMD_PLAY,
                            result
                        );

                        break;
                    }

                    case VSP_CMD_GET_DIAGNOSTICS:
                    {
                        if (header.payloadLength != 0U)
                        {
                            VSP_SendResponse(&header, VSP_CMD_GET_DIAGNOSTICS, -1);
                        }
                        else
                        {
                            VSP_SendDiagnostics(&header);
                        }
                        break;
                    }

                    default:
                    {
                        VSP_SendResponse(
                            &header,
                            header.command,
                            -1
                        );
                        break;
                    }
                }
            }
        }
/* USER CODE END P2PS_STM_WRITE_EVT */
      break;

    default:
/* USER CODE BEGIN P2PS_STM_App_Notification_default */

/* USER CODE END P2PS_STM_App_Notification_default */
      break;
  }
/* USER CODE BEGIN P2PS_STM_App_Notification_2 */

/* USER CODE END P2PS_STM_App_Notification_2 */
  return;
}

void P2PS_APP_Notification(P2PS_APP_ConnHandle_Not_evt_t *pNotification)
{
/* USER CODE BEGIN P2PS_APP_Notification_1 */

/* USER CODE END P2PS_APP_Notification_1 */
  switch(pNotification->P2P_Evt_Opcode)
  {
/* USER CODE BEGIN P2PS_APP_Notification_P2P_Evt_Opcode */

/* USER CODE END P2PS_APP_Notification_P2P_Evt_Opcode */
  case PEER_CONN_HANDLE_EVT :
/* USER CODE BEGIN PEER_CONN_HANDLE_EVT */

/* USER CODE END PEER_CONN_HANDLE_EVT */
    break;

    case PEER_DISCON_HANDLE_EVT :
/* USER CODE BEGIN PEER_DISCON_HANDLE_EVT */

/* USER CODE END PEER_DISCON_HANDLE_EVT */
    break;

    default:
/* USER CODE BEGIN P2PS_APP_Notification_default */

/* USER CODE END P2PS_APP_Notification_default */
      break;
  }
/* USER CODE BEGIN P2PS_APP_Notification_2 */

/* USER CODE END P2PS_APP_Notification_2 */
  return;
}

void P2PS_APP_Init(void)
{
/* USER CODE BEGIN P2PS_APP_Init */
  VSP_Profile_Init(&vspProfile);
  VSP_Playback_Init(&vspPlayback);
  UTIL_SEQ_RegTask(
    1UL << CFG_TASK_VSP_PLAYBACK_ID,
    UTIL_SEQ_RFU,
    VSP_Playback_Task
);

/* USER CODE END P2PS_APP_Init */
  return;
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/

/*
 * Send a VSP response on FE42.
 *
 * Payload:
 *   Byte 0: Status (0 = success, nonzero = error)
 *   Byte 1: Reserved
 *
 * The response retains the request's transfer ID and sequence.
 */
static uint32_t VSP_ReadTimestampUs(uint32_t sampleIndex)
{
    uint32_t offset =
        VSP_FORMAT_HEADER_SIZE +
        sampleIndex * VSP_FORMAT_RECORD_SIZE;

    const uint8_t *record = &vspProfile.data[offset];

    return (uint32_t)record[0] |
           ((uint32_t)record[1] << 8) |
           ((uint32_t)record[2] << 16) |
           ((uint32_t)record[3] << 24);
}

static void VSP_Playback_Task(void)
{
    static volatile uint32_t executionCount = 0U;

    executionCount++;

    uint32_t nowUs = __HAL_TIM_GET_COUNTER(&htim2);
    uint16_t dacCode = 0U;

    uint32_t previousSample = vspPlayback.nextSample;

    int result = VSP_Playback_Update(
        &vspPlayback,
        nowUs,
        &dacCode
    );

    if (result > 0)
    {
        lastDacCode = dacCode;

        uint32_t currentSample = vspPlayback.nextSample;

        if (previousSample == 0U && currentSample == 1U)
        {
            sampleTimeUs[0] = nowUs;
        }
        else if (previousSample == 1U && currentSample == 2U)
        {
            sampleTimeUs[1] = nowUs;
            sampleIntervalUs = sampleTimeUs[1] - sampleTimeUs[0];
        }
    }

    if (vspPlayback.state == VSP_PLAYBACK_RUNNING &&
        vspPlayback.nextSample < vspPlayback.sampleCount)
    {
        uint32_t nextTimestampUs =
            VSP_ReadTimestampUs(vspPlayback.nextSample);

        uint32_t deadlineUs =
            vspPlayback.startTimeUs + nextTimestampUs;

        /*
         * Arm TIM2 Channel 1 for the next sample.
         * If the deadline has already passed, request
         * another update immediately.
         */
        uint32_t currentUs = __HAL_TIM_GET_COUNTER(&htim2);

        if ((int32_t)(deadlineUs - currentUs) <= 0)
        {
            UTIL_SEQ_SetTask(
                1UL << CFG_TASK_VSP_PLAYBACK_ID,
                CFG_SCH_PRIO_0
            );
        }
        else
        {
            __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);

            __HAL_TIM_SET_COMPARE(
                &htim2,
                TIM_CHANNEL_1,
                deadlineUs
            );

            __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);

            __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC1);

            /*
            * Check whether the deadline passed while
            * we were configuring the compare register.
            */
            uint32_t afterArmUs =
                __HAL_TIM_GET_COUNTER(&htim2);

            if ((int32_t)(deadlineUs - afterArmUs) <= 0)
            {
                __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);

                UTIL_SEQ_SetTask(
                    1UL << CFG_TASK_VSP_PLAYBACK_ID,
                    CFG_SCH_PRIO_0
                );
            }
        }
    }
    else
    {
        __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);
    }
}

/* Diagnostics payload (25 bytes, little-endian):
 *  0..1 status (uint16, 0 = success)
 *  2    playback state (uint8)
 *  3..6 nextSample (uint32)
 *  7..10 TIM2 compare IRQ count (uint32)
 * 11..14 first sample processing time (uint32, us)
 * 15..18 second sample processing time (uint32, us)
 * 19..22 measured sample interval (uint32, us)
 * 23..24 final calculated DAC code (uint16)
 */
static void VSP_WriteU16LE(uint8_t *dest, uint16_t value)
{
    dest[0] = (uint8_t)value;
    dest[1] = (uint8_t)(value >> 8);
}

static void VSP_WriteU32LE(uint8_t *dest, uint32_t value)
{
    dest[0] = (uint8_t)value;
    dest[1] = (uint8_t)(value >> 8);
    dest[2] = (uint8_t)(value >> 16);
    dest[3] = (uint8_t)(value >> 24);
}

static void VSP_SendDiagnostics(const VSP_Header_t *request)
{
    uint8_t payload[VSP_DIAGNOSTICS_PAYLOAD_SIZE] = {0};
    VSP_WriteU16LE(&payload[0], 0U);
    payload[2] = (uint8_t)vspPlayback.state;
    VSP_WriteU32LE(&payload[3], vspPlayback.nextSample);
    VSP_WriteU32LE(&payload[7], tim2CompareCount);
    VSP_WriteU32LE(&payload[11], sampleTimeUs[0]);
    VSP_WriteU32LE(&payload[15], sampleTimeUs[1]);
    VSP_WriteU32LE(&payload[19], sampleIntervalUs);
    VSP_WriteU16LE(&payload[23], lastDacCode);

    VSP_Header_t responseHeader = {
        .version = VSP_VERSION_MAJOR,
        .command = VSP_CMD_GET_DIAGNOSTICS,
        .flags = 0U,
        .headerLength = VSP_HEADER_SIZE,
        .transferId = request->transferId,
        .sequence = request->sequence,
        .payloadLength = sizeof(payload)
    };
    uint8_t packet[VSP_HEADER_SIZE + VSP_DIAGNOSTICS_PAYLOAD_SIZE];
    uint16_t packetLength = 0U;
    if (VSP_EncodePacket(&responseHeader, payload, packet,
                         sizeof(packet), &packetLength) == 0)
    {
        (void)P2PS_STM_App_Update_Char(
            P2P_NOTIFY_CHAR_UUID, packet, (uint8_t)packetLength);
    }
}

static void VSP_SendResponse(
    const VSP_Header_t *request,
    uint8_t command,
    int status)
{
    uint8_t payload[2] =
    {
        (uint8_t)(status == 0 ? 0 : -status),
        0U
    };

    VSP_Header_t responseHeader =
    {
        .version = VSP_VERSION_MAJOR,
        .command = command,
        .flags = 0U,
        .headerLength = VSP_HEADER_SIZE,
        .transferId = request->transferId,
        .sequence = request->sequence,
        .payloadLength = sizeof(payload)
    };

    uint8_t packet[VSP_HEADER_SIZE + sizeof(payload)];
    uint16_t packetLength = 0U;

    if (VSP_EncodePacket(
            &responseHeader,
            payload,
            packet,
            sizeof(packet),
            &packetLength) == 0)
    {
        (void)P2PS_STM_App_Update_Char(
            P2P_NOTIFY_CHAR_UUID,
            packet,
            (uint8_t)packetLength
        );
    }
}

/* USER CODE END FD_LOCAL_FUNCTIONS*/
