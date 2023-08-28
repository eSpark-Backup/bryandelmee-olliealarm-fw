# Noozle Project

Welcome to the Noozle project repository!

## BLEProv Initialization

Before you can make use of the BLEProv module, certain prerequisites must be met. Ensure the following initializations are performed in your code:

1. **nvs_flash_init():** Initialize the Non-Volatile Storage (NVS) Flash. This is necessary for storing configuration data.

2. **esp_netif_init():** Initialize the ESP-IDF network interface stack. This is important for network-related operations.

3. **esp_event_loop_create_default():** Create the default event loop for handling ESP-IDF events. This event loop is essential for managing asynchronous events within your application.

4. **Wi-Fi Initialization:** Ensure that your Wi-Fi module is initialized correctly if your application relies on Wi-Fi connectivity.

## BLEProv::setup()

The `BLEProv::setup()` function is a crucial part of your BLE provisioning setup. This function performs the following tasks:

- **Initialization:** It initializes the Provisioning Manager, setting up the necessary components for BLE provisioning.

- **Service Name and UUID:** You can define the service name and service UUID for your device. This information is used to identify your device during provisioning.

- **Security Mode and Proof of Possession (PoP):** Specify the security mode and the Proof of Possession (PoP) method for your device. This is essential for ensuring secure provisioning.

## BLEProv::run()

The `BLEProv::run()` function is responsible for starting the BLE provisioning process. Once you have configured the necessary parameters using `BLEProv::setup()`, you can call this function to initiate the provisioning process.

This function will advertise your device and make it discoverable to other provisioning devices, allowing them to securely configure your device.

## Example Usage

Here's an example of how you can use the BLEProv module within your code:

```cpp
#include "BLEProv.h"

void setup() {
    // Initialize necessary components
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    // Initialize Wi-Fi

    // Initialize BLE provisioning
    BLEProv::setup();

    // Start BLE provisioning
    BLEProv::run();
}

void loop() {
    // Your main loop code here
}
```

Make sure to adapt this code to your specific use case and hardware configuration.

## Getting Help

If you encounter any issues or have questions related to the Noozle project or BLE provisioning, please feel free to open an issue in this repository, and our community will be happy to assist you.

Happy provisioning!