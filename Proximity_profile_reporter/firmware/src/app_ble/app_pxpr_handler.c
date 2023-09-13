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
  Application BLE Profile Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_pxpr_handler.c

  Summary:
    This file contains the Application BLE functions for this project.

  Description:
    This file contains the Application BLE functions for this project.
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "ble_pxpr/ble_pxpr.h"
#include "config/default/peripheral/gpio/plib_gpio.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************

void APP_PxprEvtHandler(BLE_PXPR_Event_T *p_event)
{
    switch(p_event->eventId)
    {
        case BLE_PXPR_EVT_LLS_ALERT_LEVEL_WRITE_IND:
        {
            /* TODO: implement your application code.*/
            printf("\r\nLLS_ALERT_LEVEL_WRITE WRITE\r\n");
        }
        break;
        case BLE_PXPR_EVT_IAS_ALERT_LEVEL_WRITE_IND:
        {
            /* TODO: implement your application code.*/
            printf("\r\nIAS_ALERT_LEVEL_WRITE WRITE\r\n");
            uint8_t alert_lvl=p_event->eventField.evtIasAlertLevelWriteInd.alertLevel;
            RED_LED_Clear();
            BLUE_LED_Clear();
            GREEN_LED_Clear();
            if(alert_lvl==0x02)
            {
                RED_LED_Set();
            }
            else if(alert_lvl==0x00)
            {
                GREEN_LED_Set();
            }
            else
            {
                BLUE_LED_Set();
            }
            
        }
        break;

        case BLE_PXPR_EVT_ERR_UNSPECIFIED_IND:
        {
            /* TODO: implement your application code.*/
        }
        break;

        default:
        break;
    }
}