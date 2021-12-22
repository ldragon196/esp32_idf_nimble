/*
 *  main.c
 *
 *  Created on: Dec 12, 2021
 */

/******************************************************************************/

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include <nvs_flash.h>

#include "config.h"
#include "ble_api/gatt_server.h"
#include "ble_api/ble_api.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/



/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

static const char* TAG = "MAIN";

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                                FUNCTIONS                                   */
/******************************************************************************/

static void main_ble_handle_packet(uint8_t *data, size_t size);

/******************************************************************************/

/**
 * @brief  BLE data handler
 */
static void main_ble_handle_packet(uint8_t *data, size_t size)
{
    ESP_LOGI(TAG, "Received %u bytes", size);
    
    /* Send response */
    ble_api_tx_notify(data, size);
}

/******************************************************************************/

/**
 * @brief  Main app
 */
void app_main()
{
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "Power up! Firmware version %s, hardware version %s", FIRMWARE_VERSION, HARDWARE_VERSION);

    /* BLE initialization */
    ble_api_init(BLE_DEVICE_NAME, BLE_PIN_CODE);

    /* Register callback to handle data received */
    gatt_server_register_rx_handler(main_ble_handle_packet);

    while(1)
    {
        ESP_LOGI(TAG, "Free heap %u", esp_get_minimum_free_heap_size());
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}