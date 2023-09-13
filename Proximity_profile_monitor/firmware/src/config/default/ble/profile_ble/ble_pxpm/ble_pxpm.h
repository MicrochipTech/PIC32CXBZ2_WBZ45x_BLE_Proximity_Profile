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
  BLE Proximity Profile Monitor Header File

  Company:
    Microchip Technology Inc.

  File Name:
    ble_pxpm.h

  Summary:
    This file contains the BLE Proximity Profile Monitor functions for application user.

  Description:
    This file contains the BLE Proximity Profile Monitor functions for application user.
 *******************************************************************************/


/**
 * @addtogroup BLE_PXPM
 * @{
 * @brief Header file for the BLE Proximity profile Monitor/Client library.
 * @note Definitions and prototypes for the BLE Proximity profile client stack layer application programming interface.
 */
#ifndef BLE_PXPM_H
#define BLE_PXPM_H

// *****************************************************************************
// *****************************************************************************
// Section: Macros for Included Files
// *****************************************************************************
// *****************************************************************************
/**@addtogroup BLE_PXPM_SUPP_SVC_DEFINES Defines
 * @{ */

/**@defgroup BLE_PXPM_SUPP_SVC Supported service
 * @brief The definition of supported service.
 * @{ */
#define BLE_PXPM_IAS_ENABLE                     /**< Feature of PXP Monitor supporting optional Immediate Alert Service. */
#define BLE_PXPM_TPS_ENABLE                     /**< Feature of PXP Monitor supporting optional Tx Power Service. */
/** @} */

/**@} */ //BLE_PXPM_SUPP_SVC_DEFINES

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "stack_mgr.h"
#include "ble_gcm/ble_dd.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
/**@addtogroup BLE_PXPM_DEFINES Defines
 * @{ */

/**@} */ //BLE_PXPM_DEFINES

/**@addtogroup BLE_PXPM_ENUMS Enumerations
 * @{ */

/**@brief Enumeration type of BLE PXP Monitor alert levels. */
typedef enum BLE_PXPM_AlertLevel_T
{
    BLE_PXPM_ALERT_LEVEL_NO = 0x00,                 /**< Indicate alert level for No Alert. */
    BLE_PXPM_ALERT_LEVEL_MILD = 0x01,               /**< Indicate alert level for Mild Alert. */
    BLE_PXPM_ALERT_LEVEL_HIGH = 0x02,               /**< Indicate alert level for High Alert. */
} BLE_PXPM_AlertLevel_T;


/**@brief Enumeration type of BLE PXP Monitor callback events. */
typedef enum BLE_PXPM_EventId_T
{
    BLE_PXPM_EVT_DISC_COMPLETE_IND = 0x00,          /**< PXP Reporter Service discovery completed and ready to perform PXP procedure. See @ref BLE_PXPM_EvtDiscComplete_T for event details.*/
    BLE_PXPM_EVT_LLS_ALERT_LEVEL_WRITE_RSP_IND,     /**< Event for receiving LLS Alert Level write response. See @ref BLE_PXPM_EvtLlsAlertLvWriteRspInd_T for event details.*/
    BLE_PXPM_EVT_LLS_ALERT_LEVEL_IND,               /**< Event for receiving LLS Alert Level read response. See @ref BLE_PXPM_EvtLlsAlertLvInd_T for event details.*/
    BLE_PXPM_EVT_TPS_TX_POWER_LEVEL_IND,            /**< Event for receiving TPS TX Power Level read response. See @ref BLE_PXPM_EvtTpsTxPwrLvInd_T for event details.*/
    BLE_PXPM_EVT_ERR_UNSPECIFIED_IND                /**< Profile internal unspecified error occurs. */
}BLE_PXPM_EventId_T;

/**@} */ //BLE_PXPM_ENUMS

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
/**@addtogroup BLE_PXPM_STRUCTS Structures
 * @{ */

/**@brief Data structure for @ref BLE_PXPM_EVT_DISC_COMPLETE_IND event. */
typedef struct BLE_PXPM_EvtDiscComplete_T
{
    uint16_t        connHandle;         /**< The connection handle of discovery completion. */
} BLE_PXPM_EvtDiscComplete_T;


/**@brief Data structure for @ref BLE_PXPM_EVT_LLS_ALERT_LEVEL_WRITE_RSP_IND event. */
typedef struct BLE_PXPM_EvtLlsAlertLvWriteRspInd_T
{
    uint16_t        connHandle;         /**< Connection Handle */
    uint16_t        errCode;            /**< The error code of the writing alert level response. See @ref ATT_ERROR_CODES.*/
} BLE_PXPM_EvtLlsAlertLvWriteRspInd_T;


/**@brief Data structure for @ref BLE_PXPM_EVT_TPS_TX_POWER_LEVEL_IND event. */
typedef struct BLE_PXPM_EvtTpsTxPwrLvInd_T
{
    uint16_t        connHandle;         /**< Connection handle */
    int8_t          txPowerLevel;       /**< Tx power level */
} BLE_PXPM_EvtTpsTxPwrLvInd_T;


/**@brief Data structure for @ref BLE_PXPM_EVT_LLS_ALERT_LEVEL_IND event. */
typedef struct BLE_PXPM_EvtLlsAlertLvInd_T
{
    uint16_t                connHandle;         /**< Connection handle */
    BLE_PXPM_AlertLevel_T   alertLevel;         /**< Alert level*/
} BLE_PXPM_EvtLlsAlertLvInd_T;


/**@brief Union of BLE PXP Monitor callback event data types. */
typedef union
{
    BLE_PXPM_EvtDiscComplete_T          evtDiscComplete;                /**< Handle @ref BLE_PXPM_EVT_DISC_COMPLETE_IND */
    BLE_PXPM_EvtLlsAlertLvWriteRspInd_T evtLlsAlertLvWriteRspInd;       /**< Handle @ref BLE_PXPM_EVT_LLS_ALERT_LEVEL_WRITE_RSP_IND */
    BLE_PXPM_EvtLlsAlertLvInd_T         evtLlsAlertLvInd;               /**< Handle @ref BLE_PXPM_EVT_LLS_ALERT_LEVEL_IND */
    BLE_PXPM_EvtTpsTxPwrLvInd_T         evtTpsTxPwrLvInd;               /**< Handle @ref BLE_PXPM_EVT_TPS_TX_POWER_LEVEL_IND */
} BLE_PXPM_EventField_T;


/**@brief BLE PXP Monitor callback event. */
typedef struct  BLE_PXPM_Event_T
{
    BLE_PXPM_EventId_T                  eventId;            /**< Event ID. See @ref BLE_PXPM_EventId_T.  */
    BLE_PXPM_EventField_T               eventField;         /**< Event field */
} BLE_PXPM_Event_T;

/**@brief BLE PXP Monitor callback type. This callback function sends BLE PXP Monitor events to the application. */
typedef void(*BLE_PXPM_EventCb_T)(BLE_PXPM_Event_T *p_event);

/**@} */ //BLE_PXPM_STRUCTS

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
/**@addtogroup BLE_PXPM_FUNS Functions
 * @{ */

/**
 * @brief Initialize BLE PXP monitor.
 *
 * @retval MBA_RES_SUCCESS          Success to initalize BLE PXP monitor.
 * @retval MBA_RES_FAIL             Fail to initalize BLE PXP monitor.
 *
 */
uint16_t BLE_PXPM_Init(void);


/**
 * @brief Register BLE PXP Monitor callback.\n
 *       Application must call this API after starting BLE PXP Monitor procedure.
 *
 * @param[in] routine        Client callback function.
 *
 */
void BLE_PXPM_EventRegister(BLE_PXPM_EventCb_T routine);


/**
 * @brief Write Alert Level value to Link Loss Service table of peer PXP Reporter device.\n
 *       Application must call this API after starting BLE PXP Monitor procedure.
 *
 * @param[in] connHandle     The connection handle want to be set.
 * @param[in] level          The alert level want to set for the connHandle.
 *
 */
uint16_t BLE_PXPM_WriteLlsAlertLevel(uint16_t connHandle, BLE_PXPM_AlertLevel_T level);


/**
 * @brief Write Alert Level value to Immediate Alert Service table of peer PXP Reporter device.\n
 *       Application must call this API after starting BLE PXP Monitor procedure.
 *
 * @param[in] connHandle     The connection handle want to be set.
 * @param[in] level          The alert level want to set for the connHandle.
 *
 */
uint16_t BLE_PXPM_WriteIasAlertLevel(uint16_t connHandle, BLE_PXPM_AlertLevel_T level);


/**
 * @brief Read Alert Level value from Link Loss Service table of peer PXP Reporter device.\n
 *       Application must call this API after starting BLE PXP Monitor procedure.
 *
 * @param[in] connHandle     The connection handle want to be set.
 *
 */
uint16_t BLE_PXPM_ReadLlsAlertLevel(uint16_t connHandle);


/**
 * @brief Read Tx Power Level value from Tx Power Service table of peer PXP Reporter device.\n
 *       Application must call this API after starting BLE PXP Monitor procedure.
 *
 * @param[in] connHandle     The connection handle want to be set.
 *
 */
uint16_t BLE_PXPM_ReadTpsTxPowerLevel(uint16_t connHandle);


/**@brief Handle BLE_Stack related events.
 *       This API should be called in the application while caching BLE_Stack events
 *
 * @param[in] p_stackEvent   Pointer to BLE_Stack events buffer.
 *
*/
void BLE_PXPM_BleEventHandler(STACK_Event_T *p_stackEvent);


/**@brief Handle BLE DD (Database Discovery middleware) events.
 *       This API should be called in the application while caching BLE DD events
 *
 * @param[in] p_event        Pointer to BLE DD events buffer.
 *
*/
void BLE_PXPM_BleDdEventHandler(BLE_DD_Event_T *p_event);


/**@} */ //BLE_PXPM_FUNS


#endif

/**
  @}
*/


