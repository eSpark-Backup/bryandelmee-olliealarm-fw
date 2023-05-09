# _WIFI PROVISIONING_

Espressif has the following document for the [Wifi Provisioning](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/provisioning/wifi_provisioning.html).

# Implementation:
- The WIFI mode will be initialize on the boot.
- Check for the saved credentials in the NVS Storage namespace(wifi_config). 
- If it doesn't found any credentials then it'll switch to the hardcode credentials(which will be replaced by BLE-Service).