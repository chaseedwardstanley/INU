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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_protocol.h"
#include "v_profile.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static VSP_Profile_t vspProfile;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void VSP_SendResponse(
    const VSP_Header_t *request,
    uint8_t command,
    int status
);
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
