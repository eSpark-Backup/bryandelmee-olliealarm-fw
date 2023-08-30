#include "wifi.hpp"
#include "esp_log.h"

static const char* TAG = "WIFI-EVENT-MODULE";

esp_err_t WIFI::wait()
{
    EventBits_t uxBits = xEventGroupWaitBits(
            event_group,   /* The event group being tested. */
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, /* The bits within the event group to wait for. */
            pdTRUE,        /* BIT_0 & BIT_1 should be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
            portMAX_DELAY);/* Wait a maximum of 100ms for either bit to be set. */

    if( ( uxBits & WIFI_CONNECTED_BIT ) != 0 )
    {
        /* xEventGroupWaitBits() returned because WIFI_CONNECTED_BIT was set. */
        return ESP_OK;
    }
    return ESP_FAIL;  /* WIFI_FAIL_BIT was set */
}

esp_err_t WIFI::setupEventGroup()
{
    event_group = xEventGroupCreate();
    if(event_group == NULL)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t WIFI::setupEventHandler()
{
    esp_err_t ret = ESP_OK;
    ret |= esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &eventHandlerWIFI, this, &instance_any_id);
    ret |= esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &eventHandlerIP, this, &instance_got_ip);
    return ret;
}

void WIFI::eventHandlerWIFI(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData)
{
    WIFI* thisObj = (WIFI*)arg;
    static uint8_t retries;

    switch (eventID)
    {
        case WIFI_EVENT_STA_START:
            thisObj->connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            (retries < 5) ? thisObj->connect() , retries++ : xEventGroupSetBits(thisObj->event_group, thisObj->WIFI_FAIL_BIT);
            break;

        case WIFI_EVENT_STA_CONNECTED:
            retries = 0;
            break;

        default:
            break;
    }
}

void WIFI::eventHandlerIP(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData) 
{
    WIFI* thisObj = (WIFI*)arg;
    switch (eventID)
    {
        case IP_EVENT_STA_GOT_IP:
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)eventData;
            ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(thisObj->event_group, thisObj->WIFI_CONNECTED_BIT);
            break;
        }
        default:
            break;
   }
}
