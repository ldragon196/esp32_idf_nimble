/*
 *  config.h
 *
 *  Created on: Oct 1, 2021
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/******************************************************************************/

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

/* BLE */
#define BLE_ADV_DURATION_MS                           BLE_HS_FOREVER
#define BLE_IO_TYPE                                   BLE_SM_IO_CAP_DISP_ONLY
#define BLE_DEVICE_NAME                               "ESP32 BLE"
#define BLE_PIN_CODE                                  123456

/* Info */
#define FIRMWARE_VERSION                              "1.0.0"
#define HARDWARE_VERSION                              "1.0.0"

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/



/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/



/******************************************************************************/
/*                                FUNCTIONS                                   */
/******************************************************************************/



/******************************************************************************/

#endif /* _CONFIG_H_ */