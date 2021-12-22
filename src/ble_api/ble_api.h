/*
 *  ble_api.h
 *
 *  Created on: Oct 1, 2021
 */

#ifndef _BLE_API_H_
#define _BLE_API_H_

/******************************************************************************/

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include <esp_bt.h>

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

#define BLE_BONDING_FLAG                              1
#define BLE_MITM_FLAG                                 1
#define BLE_USE_SC_FLAG                               1

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/



/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/



/******************************************************************************/
/*                                FUNCTIONS                                   */
/******************************************************************************/

/**
 * @brief  Enables advertising with the following parameters:
 *             General discoverable mode
 *             Undirected connectable mode
 * @param  None
 * @retval None
 */
void ble_api_advertise(void);

/**
 * @brief  Setup local mtu that will be used to negotiate mtu during request from client peer
 * @param  MTU (ATT Maximum Transmission Unit) value
 * @retval None
 */
void ble_api_set_mtu(uint16_t mtu);

/**
 * @brief  Send tx data via ble
 * @param  data   : buffer hold data to send
 *         length : length of data (max is BLE_UART_MAX_MTU)
 * @retval None
 */
void ble_api_tx_notify(uint8_t *data, size_t size);

/**
 * @brief  Init the ble and make it visible
 * @param  dev_name    : device name
 *         pin         : pairing pin, pin must be a number have 6 character
 *                       otherwise not use pin
 * @retval true when init success, otherwise return false
 */
void ble_api_init(const char *dev_name, uint32_t pin_code);

/******************************************************************************/

#endif /* _BLE_API_H_ */