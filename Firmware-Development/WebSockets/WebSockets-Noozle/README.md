
---

# WebSockets on ESP32

The WebSocket component is not directly maintained in the ESP-IDF. To add the `esp_websocket_client` component, follow these steps:

1. Open your terminal and run the following command:

   ```bash
   idf.py add-dependency "espressif/esp_websocket_client^1.0.1"
   ```

   Source: [esp_websocket_client on Espressif Components](https://components.espressif.com/components/espressif/esp_websocket_client)

2. To verify that the WebSocket client is working correctly on your ESP32 board, we recommend using the Postman WebSocket Echo API at `wss://ws.postman-echo.com/raw`. More information can be found [here](https://blog.postman.com/introducing-postman-websocket-echo-service/).

3. Additionally, make sure to include the following line in the `CMakeLists.txt` file of your project to use `example_connect()`:

   ```cmake
   set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)
   ```

### SDK Configurations

To run the example successfully, you need to configure the following settings in your ESP-IDF:

- `CONFIG_ESP_TLS_INSECURE=y`
- `CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY=y`
- `CONFIG_EXAMPLE_WIFI_SSID=""`
- `CONFIG_EXAMPLE_WIFI_PASSWORD=""`

Make sure to set these configurations in your project's `sdkconfig` file or use `idf.py menuconfig` to access the ESP-IDF menu configuration system and set these options there.

---
