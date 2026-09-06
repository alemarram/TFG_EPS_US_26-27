# Estación Meteorológica IoT basada en ESP32

Sistema de monitorización meteorológica desarrollado como Proyecto de Fin de Grado de Ingeniería Electrónica Industrial.

El proyecto consiste en el diseño e implementación de una estación meteorológica capaz de adquirir diferentes parámetros ambientales mediante un **ESP32**, procesarlos y transmitirlos mediante una red WiFi local utilizando el protocolo **MQTT**. Los datos son gestionados mediante un broker **Mosquitto** y visualizados en tiempo real a través de un dashboard desarrollado con **Node-RED**.

## 📌 Características

El sistema permite realizar la medida y monitorización de diferentes variables ambientales:

* 🌡️ Temperatura
* 💧 Humedad relativa
* 💨 Velocidad del viento
* 🧭 Dirección del viento
* ☀️ Radiación solar
* 🌧️ Precipitación
* 🌫️ Concentración de partículas en suspensión (PM1.0, PM2.5 y PM10)

Los datos adquiridos por el ESP32 se procesan y transmiten a través de MQTT para su posterior visualización y almacenamiento.

## 🏗️ Arquitectura del sistema

```text
                   ┌─────────────────────┐
                   │     Sensores        │
                   │                     │
                   │ Temperatura/Humedad │
                   │ Viento              │
                   │ Dirección viento    │
                   │ Radiación solar     │
                   │ Pluviómetro         │
                   │ Partículas PM       │
                   └──────────┬──────────┘
                              │
                       RS-485 / Modbus
                              │
                              ▼
                   ┌─────────────────────┐
                   │        ESP32        │
                   │                     │
                   │ Adquisición de datos│
                   │ Procesamiento       │
                   │ Gestión WiFi        │
                   │ Comunicación MQTT    │
                   └──────────┬──────────┘
                              │
                           WiFi
                              │
                              ▼
                   ┌─────────────────────┐
                   │  Broker MQTT        │
                   │     Mosquitto       │
                   └──────────┬──────────┘
                              │
                           MQTT
                              │
                              ▼
                   ┌─────────────────────┐
                   │      Node-RED       │
                   │                     │
                   │ Procesamiento       │
                   │ Dashboard           │
                   │ Visualización       │
                   └─────────────────────┘
```

## 🔧 Hardware

### Controlador

* ESP32 WROOM-32
* Módulo MAX485 para comunicación RS-485
* Fuente de alimentación de 12 V
* Reguladores DC-DC para obtener las tensiones necesarias

### Sensores

| Parámetro             | Sensor            |
| --------------------- | ----------------- |
| Temperatura y humedad | CWS19-X-RS-G4     |
| Velocidad del viento  | PR-3000-FSJT-N01  |
| Dirección del viento  | PR-3000-FXJT-N01  |
| Radiación solar       | RA-N01-JT         |
| Precipitación         | Misol SP-RG       |
| Partículas            | Plantower PMS5003 |

## 💻 Software

El firmware del ESP32 está desarrollado utilizando **Arduino IDE** y lenguaje **C/C++**.

Las principales tecnologías utilizadas son:

* Arduino IDE
* C/C++
* ESP32
* MQTT
* Mosquitto
* Node-RED
* Modbus RTU
* RS-485
* WiFi
* SPIFFS

## 📡 Comunicaciones

La comunicación entre los sensores y el ESP32 se realiza mediante **RS-485 utilizando el protocolo Modbus RTU**.

El ESP32 actúa como dispositivo maestro, realizando consultas periódicas a los sensores y procesando las respuestas recibidas.

Una vez obtenidos los datos, el ESP32 se conecta a la red WiFi y publica la información mediante **MQTT**.

Los mensajes MQTT se organizan mediante diferentes topics según el tipo de información transmitida.

Ejemplo:

```text
meteo/ambiente
meteo/viento
meteo/lluvia
meteo/solar
meteo/pms
```

## 📊 Visualización

Los datos enviados mediante MQTT son recibidos por **Node-RED**, donde se procesan y se muestran mediante un dashboard.

El panel permite consultar:

* Valores actuales de los sensores.
* Evolución temporal de las variables.
* Velocidad y dirección del viento.
* Rosa de los vientos.
* Precipitación acumulada.
* Concentración de partículas.
* Radiación solar.

## ⚙️ Configuración

El sistema incorpora un sistema de configuración para facilitar la conexión del ESP32 a una red WiFi.

Los principales parámetros configurables son:

* SSID de la red WiFi.
* Contraseña WiFi.
* Nombre del dispositivo.
* Dirección IP del broker MQTT.
* Puerto MQTT.

La configuración se almacena en la memoria interna del ESP32 para conservarla después de un reinicio.


## 🚀 Puesta en marcha

### 1. Preparar el ESP32

Instalar Arduino IDE y las librerías necesarias para compilar el firmware.

Entre las principales dependencias se encuentran:

* `ModbusMaster`
* `PubSubClient`
* `ArduinoJson`
* `WiFiManager`
* Librería para el sensor PMS5003

### 2. Configurar el broker MQTT

Instalar y configurar Mosquitto en el ordenador que actuará como broker MQTT.

El ESP32 debe utilizar la dirección IP del ordenador dentro de la red local para establecer la conexión.

### 3. Configurar Node-RED

Importar el flujo disponible en:

```text
node-red/flows.json
```

Posteriormente, configurar el broker MQTT utilizado por los nodos MQTT de Node-RED.

### 4. Programar el ESP32

Abrir el proyecto mediante Arduino IDE, seleccionar la placa correspondiente al **ESP32 WROOM-32** y cargar el firmware.

Una vez iniciado, el dispositivo realizará la conexión WiFi y posteriormente establecerá la conexión con el broker MQTT.

## 🔄 Flujo de funcionamiento

```text
Lectura de sensores
        ↓
Procesamiento de datos
        ↓
ESP32
        ↓
WiFi
        ↓
Broker MQTT
        ↓
Node-RED
        ↓
Dashboard
```


