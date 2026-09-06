#include "ConfigManager.h"

#include <SPIFFS.h>
#include <ArduinoJson.h>

ConfigManager::ConfigManager()
{
    memset(&_config, 0, sizeof(_config));

    strlcpy(_config.broker,"10.1.10.10",sizeof(_config.broker));

    _config.port = 1883;
}

bool ConfigManager::begin()
{
    if (!SPIFFS.begin(true))
    {
        return false;
    }

    return true;
}

bool ConfigManager::exists()
{
    return SPIFFS.exists(_filename);
}

Config& ConfigManager::getConfig()
{
    return _config;
}

bool ConfigManager::load()
{
    if (!exists())
    {
        Serial.println("No existe archivo de configuración");
        return false;
    }

    File file = SPIFFS.open(_filename, "r");

    if (!file)
    {
        Serial.println("No se pudo abrir el archivo de configuración");
        return false;
    }

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, file);

    file.close();

    if (error)
    {
        Serial.println("Error leyendo el archivo JSON");
        return false;
    }

    strlcpy(_config.ssid,
            doc["ssid"] | "",
            sizeof(_config.ssid));

    strlcpy(_config.wifiPassword,
            doc["wifiPassword"] | "",
            sizeof(_config.wifiPassword));

    strlcpy(_config.broker,
            doc["broker"] | "",
            sizeof(_config.broker));

    _config.port = doc["port"] | 1883;

    strlcpy(_config.mqttUser,
            doc["mqttUser"] | "",
            sizeof(_config.mqttUser));

    strlcpy(_config.mqttPassword,
            doc["mqttPassword"] | "",
            sizeof(_config.mqttPassword));

    Serial.println("Configuración cargada correctamente");

    return true;
}

bool ConfigManager::save()
{
    File file = SPIFFS.open(_filename, "w");

    if (!file)
    {
        Serial.println("No se pudo crear el archivo de configuración");
        return false;
    }

    JsonDocument doc;

    doc["ssid"] = _config.ssid;
    doc["wifiPassword"] = _config.wifiPassword;

    doc["broker"] = _config.broker;
    doc["port"] = _config.port;

    doc["mqttUser"] = _config.mqttUser;
    doc["mqttPassword"] = _config.mqttPassword;

    size_t bytes = serializeJsonPretty(doc, file);

    file.close();

    if (bytes == 0)
    {
        Serial.println("Error guardando la configuración");
        return false;
    }

    Serial.println("Configuración guardada correctamente");

    return true;
}

void ConfigManager::clear()
{
    if (exists())
    {
        SPIFFS.remove(_filename);
    }

    memset(&_config, 0, sizeof(_config));

    strlcpy(_config.broker,"10.1.10.10",sizeof(_config.broker));

    _config.port = 1883;
}