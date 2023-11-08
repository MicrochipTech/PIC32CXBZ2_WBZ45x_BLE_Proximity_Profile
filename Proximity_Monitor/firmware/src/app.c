// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include <stdio.h>
#include "app.h"
#include "definitions.h"
#include "app_ble.h"
#include "ble_pxpm/ble_pxpm.h"
#include "ble_tps/ble_tps.h"
#include "ble_ias/ble_ias.h"
#include "ble_lls/ble_lls.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
extern uint16_t conn_hdl;
extern uint8_t  zone_Entered;
uint8_t cnt;
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/
APP_DATA appData;
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
BLE_PXPM_AlertLevel_T current_zone=BLE_PXPM_ALERT_LEVEL_NO;
BLE_PXPM_AlertLevel_T level=BLE_PXPM_ALERT_LEVEL_NO;
/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
void IAS_update(uint16_t conn_handle,uint8_t  alert_level)
{
    if(current_zone!=alert_level)
    {
        current_zone=alert_level;
        BLE_PXPM_WriteIasAlertLevel(conn_handle,current_zone);
        printf("Zone Entered:%d\r\n",alert_level);
    }

}
/* TODO:  Add any necessary local functions.
*/
void user_btn_cb( uintptr_t context)
{
   cnt++;
   if(cnt>2)
   {
       cnt=0;
   }
   level=cnt;
   BLE_PXPM_WriteLlsAlertLevel(conn_hdl,level);
   printf("LLS level:%d\r\n",cnt);
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;


    appData.appQueue = xQueueCreate( 64, sizeof(APP_Msg_T) );
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    APP_Msg_T    appMsg[1];
    APP_Msg_T   *p_appMsg;
    p_appMsg=appMsg;
    uint16_t result;
    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
            APP_BleStackInit();
            RTC_Timer32Start();
            BLE_IAS_Add();
            BLE_LLS_Add();
            BLE_TPS_Add();
            BLE_GAP_SetScanningEnable(true, BLE_GAP_SCAN_FD_ENABLE, BLE_GAP_SCAN_MODE_OBSERVER, 1000);
            printf("[BLE] Started Scanning!!!\r\n");
            EIC_CallbackRegister(EIC_PIN_0,user_btn_cb,0);
            if (appInitialized)
            {
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            if (OSAL_QUEUE_Receive(&appData.appQueue, &appMsg, OSAL_WAIT_FOREVER))
            {
                if(p_appMsg->msgId==APP_MSG_BLE_STACK_EVT)
                {
                    // Pass BLE Stack Event Message to User Application for handling
                    APP_BleStackEvtHandler((STACK_Event_T *)p_appMsg->msgData);
                }
                else if(p_appMsg->msgId==APP_MSG_BLE_STACK_LOG)
                {
                    // Pass BLE LOG Event Message to User Application for handling
                    APP_BleStackLogHandler((BT_SYS_LogEvent_T *)p_appMsg->msgData);
                }
                else if(p_appMsg->msgId==APP_TIMER_ID_0_MSG)
                {
                    // Pass BLE LOG Event Message to User Application for handling
                    IAS_update(conn_hdl,zone_Entered);
                }
                else if(p_appMsg->msgId == APP_MSG_BLE_SCAN_EVT)
                {
                    uint16_t connStatus;
                    BLE_GAP_EvtAdvReport_T addrDevAddr;
                    BLE_GAP_CreateConnParams_T createConnParam_t;
                    
                        memcpy(&addrDevAddr, &p_appMsg->msgData, sizeof(BLE_GAP_EvtAdvReport_T));
                    
                        createConnParam_t.scanInterval = 0x3C; // 37.5 ms 
                        createConnParam_t.scanWindow = 0x1E; // 18.75 ms
                        createConnParam_t.filterPolicy = BLE_GAP_SCAN_FP_ACCEPT_ALL;
                        createConnParam_t.peerAddr.addrType = addrDevAddr.addr.addrType;
                        memcpy(createConnParam_t.peerAddr.addr, addrDevAddr.addr.addr, GAP_MAX_BD_ADDRESS_LEN);
                        createConnParam_t.connParams.intervalMin = 0x10; // 20ms
                        createConnParam_t.connParams.intervalMax = 0x10; // 20ms
                        createConnParam_t.connParams.latency = 0;
                        createConnParam_t.connParams.supervisionTimeout = 0x48; // 720ms
                        connStatus = BLE_GAP_CreateConnection(&createConnParam_t);
                        printf("Connecting to BLE Device: RSSI:%ddBm, ", addrDevAddr.rssi);
                        if(connStatus == MBA_RES_SUCCESS)
                        {
                            printf(" - Success\r\n");
                        }
                        else
                        {
                            printf(" - Failed: 0x%X\r\n", connStatus);
                        }
                }
                else if(p_appMsg->msgId==APP_MSG_CONNECT_CB)
                {
                    APP_Msg_T appMsg;
                   // Create an instance of the BLE_GAP_SetPathLossReportingParams_T structure
                    BLE_GAP_SetPathLossReportingParams_T params;

                        // Fill in the structure fields with appropriate values
                    params.connHandle = conn_hdl/* Set the connection handle */;
                    params.highThreshold = 55 /* Set the high threshold */;
                    params.highHysteresis = 5/* Set the high hysteresis */;
                    params.lowThreshold = 30/* Set the low threshold */;
                    params.lowHysteresis = 5/* Set the low hysteresis */;
                    params.minTimeSpent = 3/* Set the minimum time spent */;
                   result = BLE_GAP_SetPathLossReportingParams(&params);
                   if(result == 0)
                   {
                       appMsg.msgId = APP_MSG_PATHLOSS_CB;
                       
                       OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                   }
                   else
                   {
                       appMsg.msgId = APP_MSG_CONNECT_CB;
                       
                       OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                   }
                }      
               else if(p_appMsg->msgId==APP_MSG_PATHLOSS_CB)
                {
                    result = BLE_GAP_SetPathLossReportingEnable(conn_hdl, 0x01);
                } 
            }
            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
