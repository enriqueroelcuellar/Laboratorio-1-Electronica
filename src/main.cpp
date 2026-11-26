#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---- Configuración WiFi y ThingsBoard ----
const char* WIFI_SSID  = "iPhone de Pelayo";
const char* WIFI_PASS  = "Pelayo00";
const char* TB_SERVER  = "thingsboard.cloud";
const int   TB_PORT    = 1883;
const char* TB_TOKEN   = "7tuz4cbiu8af6u60nthl";

const uint8_t PIN_SENSOR = 4;            // DQ del DS18B20
const uint8_t PIN_LED = 26;              // LED con resistencia de 220 ohm
const float UMBRAL_C = 30.0;             // Umbral en grados C
const unsigned long PERIODO_MS = 1000;   // Una lectura por segundo

OneWire bus(PIN_SENSOR);
DallasTemperature sensor(&bus);
unsigned long tAnterior = 0;
bool ledEncendido = false;

WiFiClient clienteWiFi;
PubSubClient mqtt(clienteWiFi);

void conectarWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" OK");
}

void conectarThingsBoard() {
  while (!mqtt.connected()) {
    Serial.print("Conectando a ThingsBoard...");
    // El token va como usuario y la contraseña se deja vacía
    if (mqtt.connect("ESP32_DS18B20", TB_TOKEN, NULL)) {
      Serial.println(" OK");
    } else {
      Serial.print(" fallo, codigo = ");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  sensor.begin();
  Serial.println("DS18B20 listo");
  conectarWiFi();
  mqtt.setServer(TB_SERVER, TB_PORT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }
  if (!mqtt.connected()) {
    conectarThingsBoard();
  }
  mqtt.loop();

  if (millis() - tAnterior >= PERIODO_MS) {
    tAnterior = millis();
    sensor.requestTemperatures();                  // Lanza la conversion
    float temperatura = sensor.getTempCByIndex(0); // Sensor numero 0 del bus

    if (temperatura == DEVICE_DISCONNECTED_C) {
      Serial.println("Error: sensor no detectado. Revisa el cableado");
    } else {
      // El LED se enciende si la temperatura supera el umbral
      ledEncendido = temperatura > UMBRAL_C;
      digitalWrite(PIN_LED, ledEncendido ? HIGH : LOW);

      Serial.print("Temperatura: ");
      Serial.print(temperatura, 2);
      Serial.print(" C");
      Serial.println(ledEncendido ? " | LED: ON" : " | LED: OFF");

      // Enviar a ThingsBoard en formato JSON
      String datos = "{\"temperatura\":" + String(temperatura, 2) +
                     ",\"led\":" + (ledEncendido ? "true" : "false") + "}";
      mqtt.publish("v1/devices/me/telemetry", datos.c_str());
    }
  }
}