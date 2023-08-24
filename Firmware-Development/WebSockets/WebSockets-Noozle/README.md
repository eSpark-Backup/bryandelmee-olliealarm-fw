### WebSockets on ESP32:

The websockets component is not directly maintained in esp-idf, so in-order to add the esp_websocket_client component run the following command in terminal:
idf.py add-dependency "espressif/esp_websocket_client^1.0.1"

source : https://components.espressif.com/components/espressif/esp_websocket_client

The postman websocket echo api is used to verify that the websocket client is working properly on the board.
wss://ws.postman-echo.com/raw

source: https://blog.postman.com/introducing-postman-websocket-echo-service/

Also, add the following line in the CMakeLists.txt of the project, to use the example_connect() :
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

Following SDK-Configurations are required to run the example:

CONFIG_ESP_TLS_INSECURE=y
CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY=y
CONFIG_EXAMPLE_WIFI_SSID=""
CONFIG_EXAMPLE_WIFI_PASSWORD=""

