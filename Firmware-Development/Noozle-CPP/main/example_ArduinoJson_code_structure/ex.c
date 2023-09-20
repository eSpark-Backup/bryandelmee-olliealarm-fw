#pragma once
#include <ArduinoJson.h>
#include <unordered_map>
#include <cstring>

class SM_Json 
{
  public:
    template <typename... Args>
    static void create(char* json, uint8_t size, Args... args) 
    {
      StaticJsonDocument<256> doc;
  
      // Add values to the document using a fold expression
      (addValueToDoc(doc, args.first, args.second), ...);
  
      // Serialize the JSON document
      serializeJson(doc, json, size);
    }

    template <typename... Args>
    static bool parse(const char* json, Args... args) 
    {
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, json);
      if (error) 
      {
        Serial.println("Error parsing JSON");
        return false;
      }

      bool result = parseValues(doc, args...);
      return result;
    }
  
  private:
    template <typename T>
    static void addValueToDoc(JsonDocument& doc, const char* key, const T& value) 
    {
      doc[key] = value;
    }

    template <typename T, typename... Rest>
    static bool parseValues(JsonDocument& doc, T pair, Rest... rest) 
    {
      JsonObject obj = doc.as<JsonObject>();
    
      if (!obj.containsKey(pair.first)) 
      {
        Serial.print(pair.first);
        Serial.println("key not found in JSON");
        return false;
      }
    
      const char* value = obj[pair.first];
      uint8_t len = strlen(value);
      memcpy(pair.second, value, len + 1);
    
      return parseValues(doc, rest...);
    }

    // Base case of recursion to terminate the recursive function call
    static bool parseValues(JsonDocument&) { return true; }
};

// Usage

SM_Json::create(factoryInfoJson, sizeof(factoryInfoJson),
                        std::make_pair("device_model", SM_MODEL),
                        std::make_pair("mac_address", Device::macAddress),
                        std::make_pair("firmware_rev", SM_FIRMWARE_VER));    

SM_Json::create(telemetryJson, sizeof(telemetryJson),
                  std::make_pair("battery", BATTERY_HEALTH),
                  std::make_pair("macAddress", Device::macAddress));
char ssid[32];
if (SM_Json::parse(rxValue.c_str(), std::make_pair("ssid", ssid), std::make_pair("pass", pass))) 
{
  //
}