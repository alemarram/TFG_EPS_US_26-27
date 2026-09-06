#ifndef CONFIG_MANAGER_H        
#define CONFIG_MANAGER_H          

#include <Arduino.h>             
struct Config
{
    // WiFi
    char ssid[32];
    char wifiPassword[64];

    // MQTT
    char broker[64];
    uint16_t port;
    char mqttUser[32];
    char mqttPassword[32];
};

class ConfigManager
{
public:

    ConfigManager();             

    bool begin();

    bool load();

    bool save();

    bool exists();

    void clear();

    Config& getConfig();

private:

    Config _config;

    const char* _filename = "/config.json";
};

#endif