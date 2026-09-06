#include "WebPageHandler.h"
#include <WiFi.h>

WebPageHandler::WebPageHandler(WebServer& server,
                               ConfigManager& configManager)
    :
    _server(server),
    _configManager(configManager)
{
}

void WebPageHandler::begin()
{
    _server.on("/", HTTP_GET, [this]()
    {
        handleRoot();
    });

    _server.on("/save", HTTP_POST, [this]()
    {
        handleSave();
    });

    _server.begin();

    Serial.println("Servidor web iniciado");
}

void WebPageHandler::handleRoot()
{
    _server.send(200, "text/html; charset=UTF-8", createPage());
}

String WebPageHandler::createSSIDList()
{
    String html;

    int n = WiFi.scanNetworks();

    if (n == 0)
    {
        html += "<option value=''>No se encontraron redes</option>";
    }
    else
    {
        Config& cfg = _configManager.getConfig();

        for (int i = 0; i < n; i++)
        {
            html += "<option value='";
            html += WiFi.SSID(i);
            html += "'";

            if (WiFi.SSID(i) == String(cfg.ssid))
                html += " selected";

            html += ">";
            html += WiFi.SSID(i);
            html += "</option>";
        }
    }

    WiFi.scanDelete();

    return html;
}

String WebPageHandler::createPage()
{
    Config& cfg = _configManager.getConfig();

    String html;
    html.reserve(5000);

    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>Configuración ESP32</title>";

    html += "<style>";

    html += "body{";
    html += "font-family:Arial;";
    html += "background:#f5f5f5;";
    html += "margin:0;";
    html += "padding:20px;";
    html += "}";

    html += ".container{";
    html += "max-width:400px;";
    html += "margin:auto;";
    html += "background:white;";
    html += "padding:20px;";
    html += "border-radius:10px;";
    html += "box-shadow:0 0 10px rgba(0,0,0,.2);";
    html += "}";

    html += "h2{";
    html += "text-align:center;";
    html += "}";

    html += "input,select{";
    html += "width:100%;";
    html += "padding:10px;";
    html += "margin-top:5px;";
    html += "margin-bottom:15px;";
    html += "box-sizing:border-box;";
    html += "}";

    html += "button{";
    html += "width:100%;";
    html += "padding:12px;";
    html += "background:#2196F3;";
    html += "color:white;";
    html += "border:none;";
    html += "font-size:16px;";
    html += "cursor:pointer;";
    html += "}";

    html += "</style>";

    html += "</head>";

    html += "<body>";

    html += "<div class='container'>";

    html += "<h2>Configuración ESP32</h2>";

    html += "<form method='POST' action='/save'>";

    html += "<label>SSID</label>";

    html += "<select name='ssid'>";
    html += createSSIDList();
    html += "</select>";

    html += "<label>Contraseña WiFi</label>";

    html += "<input type='password' name='wifiPassword' value='";
    html += String(cfg.wifiPassword);
    html += "'>";

    html += "<label>Broker MQTT</label>";
    html += "<input name='broker' value='" + String(cfg.broker) + "'>";

    html += "<label>Puerto</label>";
    html += "<input name='port' value='" + String(cfg.port) + "'>";

    html += "<label>Usuario MQTT</label>";

    html += "<input name='mqttUser' value='";
    html += String(cfg.mqttUser);
    html += "'>";

    html += "<label>Contraseña MQTT</label>";

    html += "<input type='password' name='mqttPassword' value='";
    html += String(cfg.mqttPassword);
    html += "'>";

    html += "<button type='submit'>Guardar</button>";

    html += "</form>";

    html += "</div>";

    html += "</body>";

    html += "</html>";

    return html;
}

void WebPageHandler::handleSave()
{
    // Comprobar que todos los campos obligatorios están rellenados
    if (_server.arg("ssid").isEmpty() ||
        _server.arg("wifiPassword").isEmpty() ||
        _server.arg("broker").isEmpty() ||
        _server.arg("port").isEmpty())
    {
        _server.send(200,
                     "text/html; charset=UTF-8",
                     "<!DOCTYPE html>"
                     "<html>"
                    "<head>"
                    "<meta charset='UTF-8'>"
                    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                    "<title>Error</title>"
                    "</head>"
                    "<body style='font-family:Arial;text-align:center;padding-top:50px;'>"
                    "<h2>Faltan datos por rellenar.</h2>"
                    "<p>Debe rellenar:</p>"
                    "<ul style='display:inline-block;text-align:left;'>"
                    "<li>Red WiFi</li>"
                    "<li>Contraseña WiFi</li>"
                    "<li>Broker MQTT</li>"
                    "<li>Puerto MQTT</li>"
                    "</ul>"
                    "<p>Usuario y contraseña MQTT opcionales.</p>"
                    "<br>"
                    "<button onclick='history.back()'>Volver</button>"
                    "</body>"
                    "</html>");

        return;
    }

    Config& cfg = _configManager.getConfig();

    strlcpy(cfg.ssid,
            _server.arg("ssid").c_str(),
            sizeof(cfg.ssid));

    strlcpy(cfg.wifiPassword,
            _server.arg("wifiPassword").c_str(),
            sizeof(cfg.wifiPassword));

    strlcpy(cfg.mqttUser,
            _server.arg("mqttUser").c_str(),
            sizeof(cfg.mqttUser));

    strlcpy(cfg.mqttPassword,
            _server.arg("mqttPassword").c_str(),
            sizeof(cfg.mqttPassword));

    strlcpy(cfg.broker,
            _server.arg("broker").c_str(),
            sizeof(cfg.broker));

    cfg.port = _server.arg("port").toInt();

    bool usaAutenticacion = strlen(cfg.mqttUser) > 0;

    if (_configManager.save())
    {
        String html;

        html += "<!DOCTYPE html>";
        html += "<html><head>";
        html += "<meta charset='UTF-8'>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<title>Configuración guardada</title>";
        html += "</head>";

        html += "<body style='font-family:Arial;text-align:center;padding-top:50px;'>";

        html += "<h2>Configuración guardada correctamente.</h2>";

        if (usaAutenticacion)
        {
            html += "<p><b>MQTT:</b> Conectará mediante usuario y contraseña.</p>";

        } else {
            html += "<p><b>MQTT:</b> Conectará en modo anónimo.</p>";
        }

        html += "<br>";
        html += "<p>Reiniciando el ESP32...</p>";

        html += "</body></html>";

        _server.send(200, "text/html; charset=UTF-8", html);

        delay(2000);
        Serial.flush();
        delay(100);

        ESP.restart();
    }
}

void WebPageHandler::handleScan()
{
    _server.send(501,
                 "text/plain",
                 "Escaneo de redes no implementado.");
}