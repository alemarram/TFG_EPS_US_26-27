#include <WiFi.h>
#include <PubSubClient.h>
#include <ModbusMaster.h>
#include "PMS.h"
#include "ConfigManager.h"
#include "WiFiManager.h"

/* ===================== PINES ===================== */
#define RX1 32
#define TX1 33
#define RX2 16
#define TX2 17
#define RE_DE 4
#define PIN_PLUVIO 27

#define PIN_RESET 25

/* ===================== CONSTANTES ===================== */
#define TIEMPO_REBOTE 50       // ms
#define MM_POR_PULSO 0.3f

const uint32_t INTERVALO_VIENTO    = 2000;
const uint32_t INTERVALO_TEMPHUM   = 6000;
const uint32_t INTERVALO_SOLAR     = 8000;
const uint32_t INTERVALO_PMS       = 30000;
const uint32_t INTERVALO_LLUVIA    = 60000;

bool mqttReady = false;

WiFiClient espClient;
PubSubClient client(espClient);

/* ===================== OBJETOS ===================== */
PMS pms(Serial1);
PMS::DATA data;

ModbusMaster Viento;
ModbusMaster DirViento;
ModbusMaster TempHum;
ModbusMaster Solar;

ConfigManager configManager;
WiFiManager wifiManager(configManager);
/* ===================== TIMERS ===================== */
uint32_t tViento = 0, tTempHum = 0, tSolar = 0, tPMS = 0, tLluvia = 0;

/* ===================== PLUVIOMETRO ===================== */
volatile uint32_t pulsos = 0;
volatile uint32_t ultimoPulsoISR = 0;

/* ===================== RS485 ===================== */
void preTransmission() { 
    digitalWrite(RE_DE, HIGH);
 }
void postTransmission(){ 
    digitalWrite(RE_DE, LOW);
 }

/* ===================== ISR PLUVIO ===================== */
void IRAM_ATTR isrPluvio() {
    uint32_t Tactual = millis();
    if (Tactual - ultimoPulsoISR > TIEMPO_REBOTE) {
        pulsos++;
        ultimoPulsoISR = Tactual;
    }
}

/* ===================== MQTT ===================== */
void conectarMQTT()
{
    if (client.connected())
        return;

    Config &cfg = configManager.getConfig();

    Serial.println("Conectando a MQTT...");

    bool conectado = false;

    // Si hay usuario MQTT se autentica
    if (strlen(cfg.mqttUser) > 0)
    {
        Serial.println("Modo MQTT: autenticado");

        conectado = client.connect(
            "ESP32_METEO",
            cfg.mqttUser,
            cfg.mqttPassword
        );
    }
    // Si no hay usuario, intenta conexión anónima
    else
    {
        Serial.println("Modo MQTT: anónimo");

        conectado = client.connect("ESP32_METEO");
    }

    if (conectado)
    {
        Serial.println("MQTT conectado");

        mqttReady = true;

        Serial.println();
        Serial.println("===== SISTEMA OPERATIVO =====");
        Serial.println("WiFi conectada");
        Serial.println("MQTT conectado");

        if (strlen(cfg.mqttUser) > 0)
        {
            Serial.print("Usuario MQTT: ");
            Serial.println(cfg.mqttUser);
        }
        else
        {
            Serial.println("Conexión MQTT anónima");
        }

        Serial.println("Iniciando sensores...");
        Serial.println("=============================");
    }
    else
    {
        mqttReady = false;
    }
}

void restaurarConfiguracion()
{
    Serial.println();
    Serial.println("===== RESTABLECIENDO CONFIGURACIÓN =====");

    // Borrar configuración guardada
    configManager.clear();

    // Desconectar MQTT
    if (client.connected())
    {
        client.disconnect();
    }

    // Desconectar WiFi y borrar credenciales
    WiFi.disconnect(true, true);

    delay(500);

    Serial.println("Configuración eliminada");
    Serial.println("Reiniciando ESP32...");

    delay(1000);

    ESP.restart();
}

/* ===================== LECTURAS ===================== */
void leerViento() {
    float velocidad = -1;
    const char* direccion = "unk";

    if (Viento.readHoldingRegisters(0x0000, 1) == Viento.ku8MBSuccess){
        velocidad = Viento.getResponseBuffer(0) / 10.0;
    }
    delay(5);

    if (DirViento.readHoldingRegisters(0x0000, 1) == DirViento.ku8MBSuccess) {
        switch (DirViento.getResponseBuffer(0)) {
            case 0: direccion = "0"; 
            break;
            case 1: direccion = "45";
            break;
            case 2: direccion = "90"; 
            break;
            case 3: direccion = "135"; 
            break;
            case 4: direccion = "180"; 
            break;
            case 5: direccion = "225"; 
            break;
            case 6: direccion = "270"; 
            break;
            case 7: direccion = "315"; 
            break;
            default: direccion = ""; 
            break;
        }
    }
    delay(5);

    if (velocidad >= 0) {
        char payload[128];
        snprintf(payload, sizeof(payload),
            "{ \"velocidad\": %.2f, \"direccion\": \"%s\" }",
            velocidad, direccion
        );
        Serial.print("Publicando viento: ");
        Serial.println(payload);
        client.publish("meteo/viento", payload);
    }
}

void leerTempHum() {
    if (TempHum.readHoldingRegisters(0x0000, 2) == TempHum.ku8MBSuccess) {
        float temp = ((165.0 * TempHum.getResponseBuffer(0)) / 1650.0) - 40.0;
        float hum  = (TempHum.getResponseBuffer(1) * 100.0) / 1000.0;

        char payload[128];
        snprintf(payload, sizeof(payload),
            "{ \"temperatura\": %.2f, \"humedad\": %.2f }",
            temp, hum
        );
        Serial.print("Publicando temp/hum: ");
        Serial.println(payload);
        client.publish("meteo/ambiente", payload);
    }
    delay(5);
}


void leerSolar() {
    if (Solar.readHoldingRegisters(0x0000, 1) == Solar.ku8MBSuccess) {
        char payload[64];
        snprintf(payload, sizeof(payload),
            "{ \"radiacion\": %d }",
            Solar.getResponseBuffer(0)
        );
        Serial.print("Publicando radiacion: ");
        Serial.println(payload);
        client.publish("meteo/solar", payload);
    }
    delay(5);
}


void leerPMS() {
    if (pms.readUntil(data, 100)) {
        char payload[128];
        snprintf(payload, sizeof(payload),
            "{ \"pm1\": %d,\"pm25\": %d, \"pm10\": %d }",
            data.PM_AE_UG_1_0,
            data.PM_AE_UG_2_5,
            data.PM_AE_UG_10_0
        );
        Serial.print("Publicando PM: ");
        Serial.println(payload);
        client.publish("meteo/aire", payload);
    }
}

void calcularLluvia() {
    uint32_t P;
    noInterrupts();
    P = pulsos;
    pulsos = 0;
    interrupts();

    float lluvia = P * MM_POR_PULSO;

    char payload[64];
    snprintf(payload, sizeof(payload),
        "{ \"mm_min\": %.2f }",
        lluvia
    );
    Serial.print("Publicando lluvia: ");
    Serial.println(payload);
    client.publish("meteo/lluvia", payload);
}

/* ===================== SETUP ===================== */
void setup() {
    Serial.begin(9600);

    configManager.begin();
    wifiManager.begin();

    pinMode(RE_DE, OUTPUT);
    digitalWrite(RE_DE, LOW);

    pinMode(PIN_RESET, INPUT_PULLUP);

    Serial1.begin(9600, SERIAL_8N1, RX1, TX1);
    Serial2.begin(9600, SERIAL_8N1, RX2, TX2);

    Viento.begin(2, Serial2);
    DirViento.begin(4, Serial2);
    TempHum.begin(3, Serial2);
    Solar.begin(1, Serial2);

    Viento.preTransmission(preTransmission);
    Viento.postTransmission(postTransmission);
    DirViento.preTransmission(preTransmission);
    DirViento.postTransmission(postTransmission);
    TempHum.preTransmission(preTransmission);
    TempHum.postTransmission(postTransmission);
    Solar.preTransmission(preTransmission);
    Solar.postTransmission(postTransmission);

    pinMode(PIN_PLUVIO, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_PLUVIO), isrPluvio, FALLING);


    if (wifiManager.isConnected())
    {
        Config& cfg = configManager.getConfig();

        client.setServer(cfg.broker, cfg.port);

        Serial.println("===== SISTEMA INICIADO =====");
    } else {
    
        Serial.println("Esperando configuración desde el portal web...");
    }
}

/* ===================== LOOP ===================== */
void loop()
{
    wifiManager.loop();

    if (digitalRead(PIN_RESET) == LOW)
    {
        delay(50);    // Antirrebote

        if (digitalRead(PIN_RESET) == LOW)
        {
            restaurarConfiguracion();
        }
    }

    if (Serial.available())
    {
        String comando = Serial.readStringUntil('\n');

        comando.trim();

        if (comando.equalsIgnoreCase("reset"))
        {
            restaurarConfiguracion();
        }
    }

    if (!wifiManager.isConnected())
    {
        return;
    }

    if (!client.connected())
    {
        conectarMQTT();

        if (!client.connected())
        {
            return;
        }
    }

    client.loop();

    uint32_t Tactual = millis();

    if (Tactual - tViento >= INTERVALO_VIENTO)
    {
        tViento = Tactual;
        leerViento();
    }

    if (Tactual - tTempHum >= INTERVALO_TEMPHUM)
    {
        tTempHum = Tactual;
        leerTempHum();
    }

    if (Tactual - tSolar >= INTERVALO_SOLAR)
    {
        tSolar = Tactual;
        leerSolar();
    }

    if (Tactual - tPMS >= INTERVALO_PMS)
    {
        tPMS = Tactual;
        leerPMS();
    }

    if (Tactual - tLluvia >= INTERVALO_LLUVIA)
    {
        tLluvia = Tactual;
        calcularLluvia();
    }

}
