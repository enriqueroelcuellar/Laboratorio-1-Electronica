#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
const uint8_t PIN_SENSOR = 4; // DQ del DS18B20
const uint8_t PIN_LED = 26; // LED con resistencia de 220 ohm
const float UMBRAL_C = 28.0; // Umbral en grados C (todavía sin usar)
const unsigned long PERIODO_MS = 1000; // Una lectura por segundo
OneWire bus(PIN_SENSOR);
DallasTemperature sensor(&bus);
unsigned long tAnterior = 0;
bool ledEncendido = false;
void setup() {
Serial.begin(115200);
pinMode(PIN_LED, OUTPUT);
sensor.begin();
Serial.println("DS18B20 listo");
}
void loop() {
if (millis() - tAnterior >= PERIODO_MS) {
tAnterior = millis();
sensor.requestTemperatures(); // Lanza la conversion (espera hasta 750 ms)
float temperatura = sensor.getTempCByIndex(0); // Sensor numero 0 del bus
if (temperatura == DEVICE_DISCONNECTED_C) {
Serial.println("Error: sensor no detectado. Revisa el cableado");
} else {
Serial.print("Temperatura: ");
Serial.print(temperatura, 2);
Serial.println(" C");
if (temperatura > 30) {
  digitalWrite(PIN_LED, HIGH);   // Supera el umbral: LED encendido
        Serial.println(" | LED: ON");
      } else {
        digitalWrite(PIN_LED, LOW);    // No lo supera: LED apagado
        Serial.println(" | LED: OFF");
}
}
}
}