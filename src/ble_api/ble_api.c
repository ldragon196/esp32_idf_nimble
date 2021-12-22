/*
 *  ble_api.c
 *
 *  Created on: Oct 1, 2021
 */

/******************************************************************************/

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include <nimble/ble.h>
#include <esp_nimble_hci.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <services/gap/ble_svc_gap.h>

#include "config.h"
#include "gatt_server.h"
#include "ble_api.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/



/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

static const char* TAG = "BLE";
static uint8_t own_addr_type;
static uint32_t ble_passkey = BLE_PIN_CODE;
static uint16_t ble_conn_handle;

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/



/******************************************************************************/
/*                                FUNCTIONS                                   */
/******************************************************************************/

static void ble_api_on_reset(int32_t reason);
static void ble_api_on_sync(void);
static void ble_api_host_task(void *arg);

/******************************************************************************/

/**
 * @brief  Callback when the host resets itself
 */
static void ble_api_on_reset(int32_t reason)
{
    ESP_LOGI(TAG, "on_reset() reason %d", reason);
}

/**
 * @brief  Callback when the host and controller become synced
 */
static void ble_api_on_sync(void)
{
    esp_err_t rc;

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if(rc != ESP_OK)
    {
        ESP_LOGE(TAG, "Error determining address type; rc = %d", rc);
        return;
    }

    /* Begin advertising */
    ble_api_advertise();

    ESP_LOGI(TAG, "on_sync()");
}

/**
 * @brief  Nimble task
 */
static void ble_api_host_task(void *arg)
{
    ESP_LOGI(TAG, "BLE Host Task Started");

    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();
    nimble_port_freertos_deinit();
}

/**
 * The nimble host executes this callback when a GAP event occurs
 * The application associates a GAP event callback with each connection that forms
 *
 * @param event The type of event being signalled
 * @param arg   Application-specified argument
 * @return      0 if the application successfully handled the event, nonzero on failure
 *              The semantics of the return code is specific to the particular GAP event being signalled
 */
static int32_t ble_api_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    esp_err_t rc;

    switch(event->type)
    {
    /* A new connection was established or a connection attempt failed */
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "Connection %s, status = %d",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);
        if (event->connect.status == 0)
        {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
        }
        if(event->connect.status != 0)
        {
            /* Connection failed, resume advertising */
            ble_api_advertise();
        }
        
        ble_conn_handle = event->connect.conn_handle;
        break;
    
    /* Connection terminated, resume advertising */
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Disconnected, reason %d", event->disconnect.reason);
        ble_api_advertise();
        break;
    
    case BLE_GAP_EVENT_CONN_UPDATE:
        ESP_LOGI(TAG, "Connection updated");
        rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        assert(rc == ESP_OK);
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "Advertise complete");
        ble_api_advertise();
        break;
    
    case BLE_GAP_EVENT_ENC_CHANGE:
        ESP_LOGI(TAG, "Encryptrion change event");
        rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
        assert(rc == ESP_OK);
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(TAG, "Subcribe event");
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "MTU update event, mtu %d", event->mtu.value);
        break;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
        ESP_LOGI(TAG, "Passkey event");
        struct ble_sm_io pkey = {0};
        if(event->passkey.params.action == BLE_SM_IOACT_DISP)
        {
            pkey.action = event->passkey.params.action;
            pkey.passkey = ble_passkey;
            ESP_LOGI(TAG, "Enter passkey %d on the peer side", pkey.passkey);
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            ESP_LOGI(TAG, "ble_sm_inject_io result: %d\n", rc);
        }
        break;

    default:
        break;
    }
    return 0;
}

/******************************************************************************/

/**
 * @brief  Enables advertising with the parameters
 */
void ble_api_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    esp_err_t rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */
    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[]) {
        BLE_UUID16_INIT(UART_SERVICE_ULUID16)
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    /* Setting advertisement data */
    rc = ble_gap_adv_set_fields(&fields);
    if(rc != ESP_OK)
    {
        ESP_LOGE(TAG, "Error setting advertisement data; rc = %d", rc);
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_ADV_DURATION_MS,
                           &adv_params, ble_api_gap_event, NULL);

    if(rc != ESP_OK)
    {
        ESP_LOGE(TAG, "Error enabling advertisement; rc = %d", rc);
        return;
    }
}

/**
 * @brief  Setup local mtu that will be used to negotiate mtu during request from client peer
 */
void ble_api_set_mtu(uint16_t mtu)
{
    esp_err_t rc = ble_att_set_preferred_mtu(mtu);
    if(rc != ESP_OK)
    {
        ESP_LOGE(TAG, "Error set local mtu; rc = %d", rc);
    }
}

/**
 * @brief  Send tx data via ble
 */
void ble_api_tx_notify(uint8_t *data, size_t size)
{
    struct os_mbuf *om;
    esp_err_t rc;

    om = ble_hs_mbuf_from_flat(data, size);
    rc = ble_gattc_notify_custom(ble_conn_handle, gatt_server_get_tx_handle(), om);
    assert(rc == ESP_OK);
    ESP_LOGE(TAG, "Notify success");
}

/**
 * @brief  Init the ble and make it visible
 */
void ble_api_init(const char *dev_name, uint32_t pin_code)
{
    esp_err_t rc;
    ble_passkey = pin_code;
    
    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
    nimble_port_init();

    /* Initialize the NimBLE host configuration */
    ble_hs_cfg.reset_cb = ble_api_on_reset;
    ble_hs_cfg.sync_cb = ble_api_on_sync;
    
    ble_hs_cfg.sm_io_cap = BLE_IO_TYPE;
    ble_hs_cfg.sm_bonding = BLE_BONDING_FLAG;                 /* Security Manager Bond flag */
    ble_hs_cfg.sm_mitm = BLE_MITM_FLAG;                       /* Security Manager MITM flag */
    ble_hs_cfg.sm_sc = BLE_USE_SC_FLAG;                       /* Security Manager Secure Connections flag */

    rc = gatt_server_init();
    assert(rc == ESP_OK);

    /* Set MTU value and Tx power */
    ble_api_set_mtu(BLE_ATT_MTU_MAX);

    /* Set the device name */
    rc = ble_svc_gap_device_name_set(dev_name);
    if(rc != ESP_OK)
    {
        ESP_LOGE(TAG, "Error set device name; rc = %d", rc);
    }

    nimble_port_freertos_init(ble_api_host_task);
    ESP_LOGI(TAG, "BLE initialized");
}