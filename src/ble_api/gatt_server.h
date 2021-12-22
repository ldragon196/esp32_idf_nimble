/*
 *  gatt_server.h
 *
 *  Created on: Oct 1, 2021
 */

#ifndef _GATT_SERVER_H_
#define _GATT_SERVER_H_

/******************************************************************************/

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/



/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

#define UART_SERVICE_ULUID16                          0xFB1E
#define RX_CHARACTERISTIC_FLAGS                       \
    (BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC | BLE_GATT_CHR_F_WRITE_AUTHEN)
#define TX_CHARACTERISTIC_FLAGS                       \
    (BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC | BLE_GATT_CHR_F_READ_AUTHEN | BLE_GATT_CHR_F_NOTIFY)

typedef void (*ble_rx_data_handler_t)(uint8_t*, size_t);

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
 * @brief  Register callback to handle rx data
 * @param  Callback function
 * @retval None
 */
void gatt_server_register_rx_handler(ble_rx_data_handler_t callback);

/**
 * @brief  Get TX characteristic handle
 * @param  None
 * @retval Rx handle
 */
uint16_t gatt_server_get_tx_handle(void);

/**
 * @brief  GATT server initialization
 * @param  None
 * @retval None
 */
esp_err_t gatt_server_init(void);

/******************************************************************************/

#endif /* _GATT_SERVER_H_ */