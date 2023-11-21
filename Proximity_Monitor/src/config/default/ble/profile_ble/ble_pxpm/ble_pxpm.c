/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

/*******************************************************************************
  BLE Proximity Profile Monitor Source File

  Company:
    Microchip Technology Inc.

  File Name:
    ble_pxpm.c

  Summary:
    This file contains the BLE Proximity Profile Server functions for application user.

  Description:
    This file contains the BLE Proximity Profile Server functions for application user.
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "osal/osal_freertos.h"
#include "mba_error_defs.h"
#include "ble_gap.h"
#include "gatt.h"
#include "ble_util/byte_stream.h"
#include "ble_pxpm/ble_pxpm.h"


// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
/**@defgroup BLE_PXPM_MAX_CONN_NBR Maximum connection number
 * @brief The definition of maximum connection number.
 * @{ */
#define BLE_PXPM_MAX_CONN_NBR                  BLE_GAP_MAX_LINK_NBR    /**< Maximum allowing conncetion numbers for Proximity profile client role. */
/** @} */

#define BLE_PXPM_UUID_IMMEDIATE_ALERT_SVC       0x1802
#define BLE_PXPM_UUID_LINKLOSS_SVC              0x1803
#define BLE_PXPM_UUID_TXPOWER_SVC               0x1804
#define BLE_PXPM_UUID_ALERT_LEVEL               0x2A06
#define BLE_PXPM_UUID_TXPOWER_LEVEL             0x2A07
// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************

typedef enum BLE_PXPM_CharAlertLevelIndex_T
{
    PXPM_INDEX_CHARALERTLV = 0x00,
    PXPM_CHARALERTLV_CHAR_NUM
} BLE_PXPM_CharAlertLevelIndex_T;

typedef enum BLE_PXPM_CharTxPowerLevelIndex_T
{
    PXPM_INDEX_CHARTXPWRLV = 0x00,
    PXPM_INDEX_CHARTXPWRLVCCCD,
    PXPM_INDEX_CHARTXPWRLVCPFD,
    PXPM_CHARTXPWRLV_CHAR_NUM
} BLE_PXPM_CharTxPowerLevelIndex_T;

typedef struct BLE_PXPM_ConnList_T
{
    int8_t      connIndex;
    uint16_t    connHandle;
} BLE_PXPM_ConnList_T;

static BLE_PXPM_EventCb_T       sp_pxpmCbRoutine;

static BLE_PXPM_ConnList_T      s_pxpmConnList[BLE_PXPM_MAX_CONN_NBR];

static BLE_DD_CharInfo_T        s_pxpmLlsCharInfoList[BLE_PXPM_MAX_CONN_NBR][PXPM_CHARALERTLV_CHAR_NUM];
#ifdef BLE_PXPM_IAS_ENABLE
static BLE_DD_CharInfo_T        s_pxpmIasCharInfoList[BLE_PXPM_MAX_CONN_NBR][PXPM_CHARALERTLV_CHAR_NUM];
#endif
#ifdef BLE_PXPM_TPS_ENABLE
static BLE_DD_CharInfo_T        s_pxpmTpsCharInfoList[BLE_PXPM_MAX_CONN_NBR][PXPM_CHARTXPWRLV_CHAR_NUM];
#endif

static const uint8_t            pxpmDiscLlsUuid[] =     { UINT16_TO_BYTES(BLE_PXPM_UUID_LINKLOSS_SVC) };
#ifdef BLE_PXPM_IAS_ENABLE
static const uint8_t            pxpmDiscIasUuid[] =     { UINT16_TO_BYTES(BLE_PXPM_UUID_IMMEDIATE_ALERT_SVC) };
#endif
#ifdef BLE_PXPM_TPS_ENABLE
static const uint8_t            pxpmDiscTpsUuid[] =     { UINT16_TO_BYTES(BLE_PXPM_UUID_TXPOWER_SVC) };
#endif

static const ATT_Uuid_T         pxpmDiscCharAlertLv =       { { UINT16_TO_BYTES(BLE_PXPM_UUID_ALERT_LEVEL) }, ATT_UUID_LENGTH_2 };
#ifdef BLE_PXPM_TPS_ENABLE
static const ATT_Uuid_T         pxpmDiscCharTxPwrLv =       { { UINT16_TO_BYTES(BLE_PXPM_UUID_TXPOWER_LEVEL) }, ATT_UUID_LENGTH_2 };
static const ATT_Uuid_T         pxpmDiscCharTxpwrLvCccd =   { { UINT16_TO_BYTES(UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG) }, ATT_UUID_LENGTH_2 };
static const ATT_Uuid_T         pxpmDiscCharTxpwrLvCpfd =   { { UINT16_TO_BYTES(UUID_DESCRIPTOR_CHAR_PRE_FORMAT) }, ATT_UUID_LENGTH_2 };
#endif

static BLE_DD_DiscChar_T        pxpmAlertLv =       { &pxpmDiscCharAlertLv, 0 };
#ifdef BLE_PXPM_TPS_ENABLE
static BLE_DD_DiscChar_T        pxpmTxPwrLv =       { &pxpmDiscCharTxPwrLv, 0 };
static BLE_DD_DiscChar_T        pxpmTxpwrLvCccd =   { &pxpmDiscCharTxpwrLvCccd, CHAR_SET_DESCRIPTOR };
static BLE_DD_DiscChar_T        pxpmTxpwrLvCpfd =   { &pxpmDiscCharTxpwrLvCpfd, CHAR_SET_DESCRIPTOR };
#endif

static BLE_DD_DiscChar_T        *pxpmDiscAlertLvCharList[] =
{
    &pxpmAlertLv,           /* Alert Level Characteristic */
};

#ifdef BLE_PXPM_TPS_ENABLE
static BLE_DD_DiscChar_T        *pxpmDiscTxPwrLvCharList[] =
{
    &pxpmTxPwrLv,           /* TX Power Level Characteristic */
    &pxpmTxpwrLvCccd,       /* TX Power Level Client Characteristic Configuration Descriptor */
    &pxpmTxpwrLvCpfd,       /* TX Power Level Characteristic Presentation Format */
};
#endif

static BLE_DD_CharList_T        s_pxpmLlsCharList[BLE_PXPM_MAX_CONN_NBR];
#ifdef BLE_PXPM_IAS_ENABLE
static BLE_DD_CharList_T        s_pxpmIasCharList[BLE_PXPM_MAX_CONN_NBR];
#endif
#ifdef BLE_PXPM_TPS_ENABLE
static BLE_DD_CharList_T        s_pxpmTpsCharList[BLE_PXPM_MAX_CONN_NBR];
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************

static void ble_pxpm_InitConnList(uint8_t connIndex)
{
    memset((uint8_t *)&s_pxpmConnList[connIndex], 0, sizeof(BLE_PXPM_ConnList_T));
}

static BLE_PXPM_ConnList_T *ble_pxpm_GetConnListByHandle(uint16_t connHandle)
{
    uint8_t i;

    for(i=0; i<BLE_PXPM_MAX_CONN_NBR; i++)
    {
        if (s_pxpmConnList[i].connHandle == connHandle)
        {
            return &s_pxpmConnList[i];
        }
    }
    return NULL;
}

static BLE_PXPM_ConnList_T *ble_pxpm_GetFreeConnList(void)
{
    uint8_t i;

    for(i=0; i<BLE_PXPM_MAX_CONN_NBR; i++)
    {
        if (s_pxpmConnList[i].connHandle == 0)
        {
            s_pxpmConnList[i].connIndex = i;
            return &s_pxpmConnList[i];
        }
    }
    return NULL;
}

static void ble_pxpm_ConveyEvent(uint8_t eventId, uint8_t *p_eventField, uint8_t eventFieldLen)
{
    if(sp_pxpmCbRoutine)
    {
        BLE_PXPM_Event_T evtPara;

        evtPara.eventId = eventId;
        memcpy((uint8_t *)&evtPara.eventField, p_eventField, eventFieldLen);
        sp_pxpmCbRoutine(&evtPara);
    }
}

static void ble_pxpm_InitLlsCharList(uint8_t connIndex)
{
    memset(&s_pxpmLlsCharList[connIndex], 0x0, sizeof(BLE_DD_CharList_T));
    memset(s_pxpmLlsCharInfoList[connIndex], 0x0, sizeof(BLE_DD_CharInfo_T) * PXPM_CHARALERTLV_CHAR_NUM);
    s_pxpmLlsCharList[connIndex].p_charInfo = s_pxpmLlsCharInfoList[connIndex];
}

#ifdef BLE_PXPM_IAS_ENABLE
static void ble_pxpm_InitIasCharList(uint8_t connIndex)
{
    memset(&s_pxpmIasCharList[connIndex], 0x0, sizeof(BLE_DD_CharList_T));
    memset(s_pxpmIasCharInfoList[connIndex], 0x0, sizeof(BLE_DD_CharInfo_T) * PXPM_CHARALERTLV_CHAR_NUM);
    s_pxpmIasCharList[connIndex].p_charInfo = s_pxpmIasCharInfoList[connIndex];
}
#endif

#ifdef BLE_PXPM_TPS_ENABLE
static void ble_pxpm_InitTpsCharList(uint8_t connIndex)
{
    memset(&s_pxpmTpsCharList[connIndex], 0x0, sizeof(BLE_DD_CharList_T));
    memset(s_pxpmTpsCharInfoList[connIndex], 0x0, sizeof(BLE_DD_CharInfo_T) * PXPM_CHARTXPWRLV_CHAR_NUM);
    s_pxpmTpsCharList[connIndex].p_charInfo = s_pxpmTpsCharInfoList[connIndex];
}
#endif

static uint16_t ble_pxpm_InitAlertLvSvcDiscRegister(const uint8_t * p_uuid, BLE_DD_CharList_T * p_charList)
{
    BLE_DD_DiscSvc_T pxpDisc;

    pxpDisc.svcUuid.uuidLength = ATT_UUID_LENGTH_2;
    memcpy(pxpDisc.svcUuid.uuid, p_uuid, ATT_UUID_LENGTH_2);
    pxpDisc.p_discChars = pxpmDiscAlertLvCharList;
    pxpDisc.p_charList = p_charList;
    pxpDisc.discCharsNum = PXPM_CHARALERTLV_CHAR_NUM;
    return BLE_DD_ServiceDiscoveryRegister(&pxpDisc);
}

#ifdef BLE_PXPM_TPS_ENABLE
static uint16_t ble_pxpm_InitTxPwrLvSvcDiscRegister(const uint8_t * p_uuid, BLE_DD_CharList_T * p_charList)
{
    BLE_DD_DiscSvc_T pxpDisc;

    pxpDisc.svcUuid.uuidLength = ATT_UUID_LENGTH_2;
    memcpy(pxpDisc.svcUuid.uuid, p_uuid, ATT_UUID_LENGTH_2);
    pxpDisc.p_discChars = pxpmDiscTxPwrLvCharList;
    pxpDisc.p_charList = p_charList;
    pxpDisc.discCharsNum = PXPM_CHARTXPWRLV_CHAR_NUM;
    return BLE_DD_ServiceDiscoveryRegister(&pxpDisc);
}
#endif

static void ble_pxpm_ProcDiscComplete(BLE_DD_Event_T * p_event)
{
    BLE_PXPM_EvtDiscComplete_T evtDiscCmlt;

    evtDiscCmlt.connHandle = p_event->eventField.evtDiscResult.connHandle;
    ble_pxpm_ConveyEvent(BLE_PXPM_EVT_DISC_COMPLETE_IND, (uint8_t *) &evtDiscCmlt, sizeof(BLE_PXPM_EvtDiscComplete_T));
}

static void ble_pxpm_ProcReadResponse(GATT_Event_T *p_event)
{
    BLE_PXPM_ConnList_T *p_conn = ble_pxpm_GetConnListByHandle(p_event->eventField.onReadResp.connHandle);

    if(p_conn == NULL)
    {
        ble_pxpm_ConveyEvent(BLE_PXPM_EVT_ERR_UNSPECIFIED_IND, NULL, 0);
        return;
    }

    if(p_event->eventField.onReadResp.charHandle == s_pxpmLlsCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARALERTLV].charHandle)
    {
        uint8_t *p_value = p_event->eventField.onReadResp.readValue;
        BLE_PXPM_EvtLlsAlertLvInd_T evt;

        evt.connHandle = p_event->eventField.onReadResp.connHandle;
        
        STREAM_TO_U8(&evt.alertLevel, &p_value);
        ble_pxpm_ConveyEvent(BLE_PXPM_EVT_LLS_ALERT_LEVEL_IND, (uint8_t *) &evt, sizeof(BLE_PXPM_EvtLlsAlertLvInd_T));
    }
    #ifdef BLE_PXPM_TPS_ENABLE
    else if(p_event->eventField.onReadResp.charHandle == s_pxpmTpsCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARTXPWRLV].charHandle)
    {
        uint8_t *p_value = p_event->eventField.onReadResp.readValue;
        BLE_PXPM_EvtTpsTxPwrLvInd_T evt;

        evt.connHandle = p_event->eventField.onReadResp.connHandle;

        STREAM_TO_U8(&evt.txPowerLevel, &p_value);

        ble_pxpm_ConveyEvent(BLE_PXPM_EVT_TPS_TX_POWER_LEVEL_IND, (uint8_t *) &evt, sizeof(BLE_PXPM_EvtTpsTxPwrLvInd_T));
    }
    #endif
}

static void ble_pxpm_ProcWriteResponse(GATT_Event_T *p_event)
{
    BLE_PXPM_ConnList_T *p_conn = ble_pxpm_GetConnListByHandle(p_event->eventField.onWriteResp.connHandle);

    if(p_conn == NULL)
    {
        ble_pxpm_ConveyEvent(BLE_PXPM_EVT_ERR_UNSPECIFIED_IND, NULL, 0);
        return;
    }

    if(p_event->eventField.onWriteResp.charHandle == s_pxpmLlsCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARALERTLV].charHandle)
    {
        BLE_PXPM_EvtLlsAlertLvWriteRspInd_T evt;
        evt.connHandle = p_event->eventField.onWriteResp.connHandle;
        evt.errCode = 0x00;
        ble_pxpm_ConveyEvent(BLE_PXPM_EVT_LLS_ALERT_LEVEL_WRITE_RSP_IND, (uint8_t *) &evt, sizeof(BLE_PXPM_EvtLlsAlertLvWriteRspInd_T));
    }
}

static void ble_pxpm_ProcErrorResponse(GATT_Event_T *p_event)
{
    BLE_PXPM_ConnList_T *p_conn = ble_pxpm_GetConnListByHandle(p_event->eventField.onError.connHandle);

    if(p_conn == NULL)
    {
        ble_pxpm_ConveyEvent(BLE_PXPM_EVT_ERR_UNSPECIFIED_IND, NULL, 0);
        return;
    }

    if(p_event->eventField.onError.attrHandle == s_pxpmLlsCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARALERTLV].charHandle)
    {
        BLE_PXPM_EvtLlsAlertLvWriteRspInd_T evt;

        evt.connHandle = p_event->eventField.onError.connHandle;
        evt.errCode = p_event->eventField.onError.errCode;
        ble_pxpm_ConveyEvent(BLE_PXPM_EVT_LLS_ALERT_LEVEL_WRITE_RSP_IND, (uint8_t *) &evt, sizeof(BLE_PXPM_EvtLlsAlertLvWriteRspInd_T));
    }
}

static void ble_pxpm_GattEventProcess(GATT_Event_T *p_event)
{
    switch (p_event->eventId)
    {
        case GATTC_EVT_READ_RESP:
        {
            ble_pxpm_ProcReadResponse(p_event);
        }
        break;

        case GATTC_EVT_WRITE_RESP:
        {
            ble_pxpm_ProcWriteResponse(p_event);
        }
        break;

        case GATTC_EVT_ERROR_RESP:
        {
            ble_pxpm_ProcErrorResponse(p_event);
        }
        break;
        default:
        break;
    }
}

static void ble_pxpm_InitConnCharList(uint8_t connIndex)
{
    ble_pxpm_InitConnList(connIndex);

    ble_pxpm_InitLlsCharList(connIndex);
    #ifdef BLE_PXPM_IAS_ENABLE
    ble_pxpm_InitIasCharList(connIndex);
    #endif
    #ifdef BLE_PXPM_TPS_ENABLE
    ble_pxpm_InitTpsCharList(connIndex);
    #endif
}

static void ble_pxpm_GapEventProcess(BLE_GAP_Event_T *p_event)
{
    BLE_PXPM_ConnList_T *p_conn = NULL;

    switch(p_event->eventId)
    {
        case BLE_GAP_EVT_CONNECTED:
        {
            if ((p_event->eventField.evtConnect.status == GAP_STATUS_SUCCESS))
            {
                p_conn = ble_pxpm_GetFreeConnList();
                if (p_conn != NULL)
                {
                    p_conn->connHandle = p_event->eventField.evtConnect.connHandle;
                }
            }
        }
        break;

        case BLE_GAP_EVT_DISCONNECTED:
        {
            p_conn = ble_pxpm_GetConnListByHandle(p_event->eventField.evtDisconnect.connHandle);
            if (p_conn != NULL)
            {
                ble_pxpm_InitConnCharList(p_conn->connIndex);
            }
        }
        break;

        default:
        break;
    }
}

uint16_t BLE_PXPM_Init(void)
{
    uint8_t i;
    sp_pxpmCbRoutine = NULL;

    for(i = 0; i < BLE_PXPM_MAX_CONN_NBR; i++)
    {
        ble_pxpm_InitConnCharList(i);
    }

    if(ble_pxpm_InitAlertLvSvcDiscRegister(pxpmDiscLlsUuid, s_pxpmLlsCharList) != MBA_RES_SUCCESS)
        return MBA_RES_FAIL;
    #ifdef BLE_PXPM_IAS_ENABLE
    if(ble_pxpm_InitAlertLvSvcDiscRegister(pxpmDiscIasUuid, s_pxpmIasCharList) != MBA_RES_SUCCESS)
        return MBA_RES_FAIL;
    #endif
    #ifdef BLE_PXPM_TPS_ENABLE
    if(ble_pxpm_InitTxPwrLvSvcDiscRegister(pxpmDiscTpsUuid, s_pxpmTpsCharList) != MBA_RES_SUCCESS)
        return MBA_RES_FAIL;
    #endif
    return MBA_RES_SUCCESS;
}

void BLE_PXPM_EventRegister(BLE_PXPM_EventCb_T routine)
{
    sp_pxpmCbRoutine = routine;
}

uint16_t BLE_PXPM_WriteLlsAlertLevel(uint16_t connHandle, BLE_PXPM_AlertLevel_T level)
{
    BLE_PXPM_ConnList_T *p_conn = ble_pxpm_GetConnListByHandle(connHandle);
    uint16_t result;
    GATTC_WriteParams_T *p_writeParams;

    if(p_conn == NULL || (s_pxpmLlsCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARALERTLV].charHandle==0x0000))
    {
        return MBA_RES_INVALID_PARA;
    }

    p_writeParams = OSAL_Malloc(sizeof(GATTC_WriteParams_T));
    if (p_writeParams == NULL)
    {
        return MBA_RES_OOM;
    }

    p_writeParams->charHandle = s_pxpmLlsCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARALERTLV].charHandle;
    p_writeParams->charLength = 1;
    p_writeParams->charValue[0] = level;
    p_writeParams->writeType = ATT_WRITE_REQ;
    p_writeParams->valueOffset = 0x0000;
    result = GATTC_Write(connHandle, p_writeParams);
    OSAL_Free(p_writeParams);

    return result;
}

#ifdef BLE_PXPM_IAS_ENABLE
uint16_t BLE_PXPM_WriteIasAlertLevel(uint16_t connHandle, BLE_PXPM_AlertLevel_T level)
{
    BLE_PXPM_ConnList_T *p_conn = ble_pxpm_GetConnListByHandle(connHandle);
    uint16_t result;
    GATTC_WriteParams_T *p_writeParams;

    if(p_conn == NULL || (s_pxpmIasCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARALERTLV].charHandle==0x0000))
    {
        return MBA_RES_INVALID_PARA;
    }

    p_writeParams = OSAL_Malloc(sizeof(GATTC_WriteParams_T));
    if (p_writeParams == NULL)
    {
        return MBA_RES_OOM;
    }

    p_writeParams->charHandle = s_pxpmIasCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARALERTLV].charHandle;
    p_writeParams->charLength = 1;
    p_writeParams->charValue[0] = level;
    p_writeParams->writeType = ATT_WRITE_CMD;
    p_writeParams->valueOffset = 0x0000;
    result = GATTC_Write(connHandle, p_writeParams);
    OSAL_Free(p_writeParams);

    return result;
}
#endif

uint16_t BLE_PXPM_ReadLlsAlertLevel(uint16_t connHandle)
{
    BLE_PXPM_ConnList_T *p_conn = ble_pxpm_GetConnListByHandle(connHandle);

    if(p_conn == NULL || (s_pxpmLlsCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARALERTLV].charHandle==0x0000))
    {
        return MBA_RES_INVALID_PARA;
    }
    
    return GATTC_Read(connHandle, s_pxpmLlsCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARALERTLV].charHandle, 0);
}

#ifdef BLE_PXPM_TPS_ENABLE
uint16_t BLE_PXPM_ReadTpsTxPowerLevel(uint16_t connHandle)
{
    BLE_PXPM_ConnList_T *p_conn = ble_pxpm_GetConnListByHandle(connHandle);

    if(p_conn == NULL || (s_pxpmTpsCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARTXPWRLV].charHandle==0x0000))
    {
        return MBA_RES_INVALID_PARA;
    }

    return GATTC_Read(connHandle, s_pxpmTpsCharList[p_conn->connIndex].p_charInfo[PXPM_INDEX_CHARTXPWRLV].charHandle, 0);
}
#endif

void BLE_PXPM_BleDdEventHandler(BLE_DD_Event_T *p_event)
{
    switch (p_event->eventId)
    {
        case BLE_DD_EVT_DISC_COMPLETE:
        {
            ble_pxpm_ProcDiscComplete(p_event);
        }
        break;

        default:
        break;
    }
}

void BLE_PXPM_BleEventHandler(STACK_Event_T *p_stackEvent)
{
    switch (p_stackEvent->groupId)
    {
        case STACK_GRP_BLE_GAP:
        {
            ble_pxpm_GapEventProcess((BLE_GAP_Event_T *)p_stackEvent->p_event);
        }
        break;

        case STACK_GRP_GATT:
        {
            ble_pxpm_GattEventProcess((GATT_Event_T *)p_stackEvent->p_event);
        }
        break;

        default:
        break;
    }
}

