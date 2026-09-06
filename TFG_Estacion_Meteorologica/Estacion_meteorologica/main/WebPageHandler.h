#ifndef WEB_PAGE_HANDLER_H
#define WEB_PAGE_HANDLER_H

#include <Arduino.h>
#include <WebServer.h>

#include "ConfigManager.h"

class WebPageHandler
{
public:

    WebPageHandler(WebServer& server, ConfigManager& configManager);

    void begin();

private:

    void handleRoot();

    void handleSave();

    void handleScan();

    String createPage();

    String createSSIDList();

    WebServer& _server;

    ConfigManager& _configManager;
};

#endif