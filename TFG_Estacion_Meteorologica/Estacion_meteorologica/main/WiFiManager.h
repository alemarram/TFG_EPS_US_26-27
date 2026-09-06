#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>

#include "ConfigManager.h"

class WebPageHandler;

class WiFiManager
{
public:

    WiFiManager(ConfigManager& configManager);

    void begin();

    void loop();

    bool isConnected() const;

private:

    bool connectSTA();

    void startAP();

    void stopAP();

    ConfigManager& _configManager;

    WebServer _server;

    WebPageHandler* _webPageHandler;

    bool _apRunning = false;
};

#endif