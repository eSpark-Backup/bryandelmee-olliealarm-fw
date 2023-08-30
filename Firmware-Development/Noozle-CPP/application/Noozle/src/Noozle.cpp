#include "Noozle.hpp"
#include "esp_check.h"

#define NAME_SPACE "Noozle"
#define BOOT_KEY "Boot"
#define PROV_STATUS_KEY "Prov_Status"

static const char* TAG = "Noozle";

Noozle::Noozle()
{
    nvs = new NVS();
    wifi = new WIFI();
    ble = new BLEProv();
}

esp_err_t Noozle::setup()
{
    ESP_RETURN_ON_ERROR(setupNVS(), TAG, "setup : setupNVS()");
    /* These API must be called before using WIFI/BLE */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_RETURN_ON_ERROR(wifi->setup(), TAG, "setup : wifi->setup()");
    ESP_RETURN_ON_ERROR(ble->setup(), TAG, "setup : ble->setup()");
    return ESP_OK;
}

esp_err_t Noozle::setupNVS()
{
    ESP_RETURN_ON_ERROR(nvs->init(), TAG, "setupNVS: nvs->init()");
    /*Setup Keys in the NVS with default values*/
    ESP_RETURN_ON_ERROR(nvs->setupKeyU32(NAME_SPACE, BOOT_KEY), TAG, "setupNVS : nvs->setupKeyU32(NAME_SPACE, BOOT_KEY)");
    ESP_RETURN_ON_ERROR(nvs->setupKeyU8(NAME_SPACE, PROV_STATUS_KEY), TAG, "setupNVS :nvs->setupKeyU8(NAME_SPACE, PROV_STATUS_KEY)");
    return ESP_OK;
}

esp_err_t Noozle::connectWifi()
{
    ESP_RETURN_ON_ERROR(wifi->run(), TAG, "connectWifi : wifi->run()");
    ESP_RETURN_ON_ERROR(wifi->wait(), TAG, "connectWifi : wifi->wait()");
    return ESP_OK;
}

esp_err_t Noozle::connectBLE()
{
    ESP_RETURN_ON_ERROR(ble->run(), TAG, "connectBLE: ble->run()");
    ble->wait();
    return ESP_OK;
}

esp_err_t Noozle::setProvStatus(uint8_t status)
{
    uint8_t prov_status = status;
    ESP_LOGI(TAG,"setProvStatus = %u", prov_status);
    ESP_RETURN_ON_ERROR(nvs->writeKeyU8(NAME_SPACE, PROV_STATUS_KEY, prov_status), TAG, "setProvStatus: nvs->writeKeyU8()");
    return ESP_OK;
}

esp_err_t Noozle::getProvStatus()
{
    uint8_t prov_status;
    ESP_RETURN_ON_ERROR(nvs->readKeyU8(NAME_SPACE, PROV_STATUS_KEY, &prov_status), TAG, "getProvStatus : readKeyU8");
    ESP_LOGI(TAG,"getProvStatus = %u", prov_status);
    if(prov_status == 1)
        return ESP_FAIL;
    return ESP_OK;
}

esp_err_t Noozle::bootCountUpdate()
{
    uint32_t boot_count = 0;
    ESP_RETURN_ON_ERROR(nvs->readKeyU32(NAME_SPACE, BOOT_KEY, &boot_count), TAG, "bootCountUpdate: readKeyU32");
    boot_count++;
    ESP_RETURN_ON_ERROR(nvs->writeKeyU32(NAME_SPACE, BOOT_KEY, boot_count), TAG, "bootCountUpdate: writeKeyU32");
    ESP_LOGI(TAG, "boot count = %lu", boot_count);
    return ESP_OK;
}