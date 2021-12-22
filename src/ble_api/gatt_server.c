/*
 *  gatt_server.c
 *
 *  Created on: Oct 1, 2021
 */

/******************************************************************************/

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include <host/ble_uuid.h>
#include <host/ble_gatt.h>
#include <host/ble_hs.h>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>

#include "config.h"
#include "gatt_server.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/



/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

static const char* TAG = "GATT";
static ble_rx_data_handler_t ble_rx_data_handler = NULL;
static uint16_t tx_characteristic_handle;

/**
 * The vendor specific security test service consists of two characteristics:
 *     o random-number-generator: generates a random 32-bit number each time
 *       it is read.  This characteristic can only be read over an encrypted
 *       connection.
 *     o static-value: a single-byte characteristic that can always be read,
 *       but can only be written over an encrypted connection.
 */

/* 59462f12-9543-9999-12c8-58b459a2712d */
static const ble_uuid128_t gatt_uart_service_ulid =
    BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                     0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

/* 5c3a659e-897e-45e1-b016-007107c96df6 */
static const ble_uuid128_t gatt_rx_characteristic_ulid = 
    BLE_UUID128_INIT(0xf6, 0x6d, 0xc9, 0x07, 0x71, 0x00, 0x16, 0xb0,
                     0xe1, 0x45, 0x7e, 0x89, 0x9e, 0x65, 0x3a, 0x5c);

/* 5c3a659e-897e-45e1-b016-007107c96df7 */
static const ble_uuid128_t gatt_tx_characteristic_ulid =
    BLE_UUID128_INIT(0xf7, 0x6d, 0xc9, 0x07, 0x71, 0x00, 0x16, 0xb0,
                     0xe1, 0x45, 0x7e, 0x89, 0x9e, 0x65, 0x3a, 0x5c);

static esp_err_t gatt_server_char_access_handler(uint16_t conn_handle, uint16_t attr_handle,
                                               struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_server_services[] = {
    {
        /*** Service: Security test. */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_uart_service_ulid.u,
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                /* Rx Characteristic */
                .uuid = &gatt_rx_characteristic_ulid.u,
                .access_cb = gatt_server_char_access_handler,
                .flags = RX_CHARACTERISTIC_FLAGS,
            }, {
                /* Tx Characteristic */
                .uuid = &gatt_tx_characteristic_ulid.u,
                .access_cb = gatt_server_char_access_handler,
                .val_handle = &tx_characteristic_handle,
                .flags = TX_CHARACTERISTIC_FLAGS,
            }, {
                0, /* No more characteristics in this service. */
            }
        },
    },

    {
        0, /* No more services. */
    },
};

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/



/******************************************************************************/
/*                                FUNCTIONS                                   */
/******************************************************************************/

static esp_err_t gatt_server_char_access_handler(uint16_t conn_handle, uint16_t attr_handle,
                                               struct ble_gatt_access_ctxt *ctxt, void *arg);

/******************************************************************************/

/**
 * @brief  Callback when characteristic is accessed
 */
static esp_err_t gatt_server_char_access_handler(uint16_t conn_handle, uint16_t attr_handle,
                                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    const ble_uuid_t *uuid;
    esp_err_t rc = ESP_OK;

    /* Determine which characteristic is being accessed by examining its 128-bit UUID */
    uuid = ctxt->chr->uuid;
    if(ble_uuid_cmp(uuid, &gatt_rx_characteristic_ulid.u) == 0)
    {
        if(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
        {
            if(ble_rx_data_handler != NULL)
            {
                uint8_t buffer[BLE_ATT_ATTR_MAX_LEN];
                uint16_t length;
                ble_hs_mbuf_to_flat(ctxt->om, buffer, BLE_ATT_ATTR_MAX_LEN, &length);
                ble_rx_data_handler(buffer, length);
            }
        }
    }

    /* Tx characteristic */
    if(ble_uuid_cmp(uuid, &gatt_tx_characteristic_ulid.u) == 0)
    {
        ESP_LOGI(TAG, "Tx event %d", ctxt->op);
    }

    return rc;
}

/******************************************************************************/

/**
 * @brief  Register callback to handle rx data
 * @param  Callback function
 * @retval None
 */
void gatt_server_register_rx_handler(ble_rx_data_handler_t callback)
{
    ble_rx_data_handler = callback;
}

/**
 * @brief  Get TX characteristic handle
 */
uint16_t gatt_server_get_tx_handle(void)
{
    return tx_characteristic_handle;
}

/**
 * @brief  GATT server initialization
 */
esp_err_t gatt_server_init(void)
{
    esp_err_t rc = 0;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    /* Check services and characteristics */
    rc = ble_gatts_count_cfg(gatt_server_services);
    if(rc != ESP_OK)
    {
        return rc;
    }

    /* Add services to server */
    rc = ble_gatts_add_svcs(gatt_server_services);
    if(rc != ESP_OK)
    {
        return rc;
    }

    return rc;
}