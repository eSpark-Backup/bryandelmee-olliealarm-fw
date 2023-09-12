#include "Noozle.hpp"
#include "esp_check.h"

#define SPACE_LABEL "Device_Info"
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
    ESP_RETURN_ON_ERROR(setupNVS(), TAG, "setup : Setting up NVS");
    /* These API must be called before using WIFI/BLE */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_RETURN_ON_ERROR(wifi->setup(), TAG, "setup : Setting up Wi-Fi");
    ESP_RETURN_ON_ERROR(ble->setup(), TAG, "setup : Setting up BLE");
    return ESP_OK;
}

esp_err_t Noozle::setupNVS()
{
    ESP_RETURN_ON_ERROR(nvs->init(), TAG, "setupNVS: Initialize NVS");
    /*Setup Keys in the NVS with default values*/
    ESP_RETURN_ON_ERROR(nvs->setupKeyU32(SPACE_LABEL, BOOT_KEY), TAG, "setupNVS : Setting up U32 key in NVS");
    ESP_RETURN_ON_ERROR(nvs->setupKeyU8(SPACE_LABEL, PROV_STATUS_KEY), TAG, "setupNVS : Setting up U8 key in NVS");
    return ESP_OK;
}

esp_err_t Noozle::connectWifi()
{
    ESP_RETURN_ON_ERROR(wifi->run(), TAG, "connectWifi : Wi-Fi is started");
    ESP_RETURN_ON_ERROR(wifi->wait(), TAG, "connectWifi : Waiting For Wi-FI connect/fail");
    return ESP_OK;
}

esp_err_t Noozle::connectBLE()
{
    ESP_RETURN_ON_ERROR(ble->run(), TAG, "connectBLE: Ble is started");
    ble->wait();
    return ESP_OK;
}

esp_err_t Noozle::setProvStatus(uint8_t status)
{
    uint8_t prov_status = status;
    ESP_LOGI(TAG,"setProvStatus = %u", prov_status);
    ESP_RETURN_ON_ERROR(nvs->writeKeyU8(SPACE_LABEL, PROV_STATUS_KEY, prov_status), TAG, "setProvStatus: Writes provisioning status in NVS");
    return ESP_OK;
}

esp_err_t Noozle::getProvStatus()
{
    uint8_t prov_status;
    ESP_RETURN_ON_ERROR(nvs->readKeyU8(SPACE_LABEL, PROV_STATUS_KEY, &prov_status), TAG, "getProvStatus : Reads provisioning status from NVS");
    ESP_LOGI(TAG,"getProvStatus = %u", prov_status);
    if(prov_status == 1)
        return ESP_FAIL;
    return ESP_OK;
}

esp_err_t Noozle::bootCountUpdate()
{
    uint32_t boot_count = 0;
    ESP_RETURN_ON_ERROR(nvs->readKeyU32(SPACE_LABEL, BOOT_KEY, &boot_count), TAG, "bootCountUpdate: Read boot-count from NVS");
    boot_count++;
    ESP_RETURN_ON_ERROR(nvs->writeKeyU32(SPACE_LABEL, BOOT_KEY, boot_count), TAG, "bootCountUpdate: Writes boot-count in NVS");
    ESP_LOGI(TAG, "boot count = %lu", boot_count);
    return ESP_OK;
}

void Noozle::checkProvStatusAndRunBleOrWifi()
{
    esp_err_t ret = getProvStatus();
    if(ret == ESP_FAIL)
    {
        connectBLE();
        setProvStatus(0); /*0 : Device Provisioned */
    }
    else
    {
        esp_err_t ret = connectWifi();
        if(ret != ESP_OK)
            setProvStatus(1);  /*1 : Device is not Provisioned */
    }
}