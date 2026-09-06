#include "WiFiManager.h"
#include "WebPageHandler.h"

WiFiManager::WiFiManager(ConfigManager& configManager)
    :
    _configManager(configManager),
    _server(80)
{
    _webPageHandler = nullptr;
}

void WiFiManager::begin()
{
    Serial.println("===== GESTOR WIFI =====");

    if (_configManager.exists())
    {
        Serial.println("Configuración encontrada");

        if (_configManager.load())
        {
            if (connectSTA())
            {
                stopAP();
                return;
            }
        }
    }

    Serial.println("Iniciando portal de configuración...");
    startAP();
}

void WiFiManager::loop()
{
    if (_apRunning)
    {
        _server.handleClient();
    }
}

bool WiFiManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::connectSTA()
{
    Config& cfg = _configManager.getConfig();

    // Reinicializar completamente el WiFi
    WiFi.mode(WIFI_OFF);
    delay(500);

    WiFi.mode(WIFI_STA);
    delay(500);

    Serial.print("Conectando a: ");
    Serial.println(cfg.ssid);

    WiFi.begin(cfg.ssid, cfg.wifiPassword);

    unsigned long inicio = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");

        if (millis() - inicio > 20000)
        {
            Serial.println();
            Serial.println("No ha sido posible conectar.");

            return false;
        }
    }

    Serial.println();
    Serial.println("WiFi conectada");

    return true;
}

void WiFiManager::startAP()
{
    // Apagar completamente el WiFi
    WiFi.mode(WIFI_OFF);
    delay(500);

    // Arrancar únicamente el AP
    WiFi.mode(WIFI_AP);
    delay(500);

    if (WiFi.softAP("ESP_METEO", "12345678"))
    {
        Serial.println("Modo AP iniciado");

        _apRunning = true;

        if (_webPageHandler == nullptr)
        {
            _webPageHandler = new WebPageHandler(_server, _configManager);
            _webPageHandler->begin();
        }
    }
    else
    {
        Serial.println("Error creando el AP");
    }
}

void WiFiManager::stopAP()
{
    if (!_apRunning)
        return;

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);

    _apRunning = false;

    Serial.println("Punto de acceso detenido");
}