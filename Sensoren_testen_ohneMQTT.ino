// Einbinden der notwendigen Bibliotheken
#include <string.h>
#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <Wire.h>



// Definition von GPIO-Pins und Messintervall
const int Messintervall = 2000;  // Messintervall in Millisekunden (2 Selunden)
int sda_pin = 4;  // SDA-Pin auf den ESP32, benötigt für die I2C Kommunikation
int scl_pin = 5;  // SCL-Pin auf den ESP32, benötigt für die I2C Kommunikation



// Variablen für die Sensordaten
float massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0;
float numberConcentrationPm0p5, numberConcentrationPm1p0, numberConcentrationPm2p5, numberConcentrationPm4p0, numberConcentrationPm10p0;
float ambientHumidity, ambientTemperature, vocIndex, noxIndex;
float typicalParticleSize;


//Objekte für Sensor
SensirionI2CSen5x sen5x;



// Setup-Funktion: Wird einmal beim Start ausgeführt
void setup() {

    Serial.begin(115200); // Serielle Kommunikation starten mit 115200 Baud

    // Initialisierung des Sensors
    Wire.begin(sda_pin, scl_pin);
    sen5x.begin(Wire);
    sen5x.deviceReset();
    
    // Start der Messung und Fehlerprüfung
    int error = sen5x.startMeasurement();
    if (error) {
      Serial.print("Fehler beim Start der Messung: ");
      Serial.println(error);
  }


}

// Loop-Funktion: Wird kontinuierlich ausgeführt
void loop() {

    // Auslesen der Sensordaten
    sen5x.readMeasuredValues(massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex, noxIndex);
    sen5x.readMeasuredPmValues(massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, numberConcentrationPm0p5, numberConcentrationPm1p0, numberConcentrationPm2p5, numberConcentrationPm4p0, numberConcentrationPm10p0, typicalParticleSize);
    

    // JSON-Datenstring für MQTT vorbereiten
    String payload = "{\n";
    payload += "  Humidity: " + String(ambientHumidity) + " %,\n";
    payload += "  Temperature: " + String(ambientTemperature) + " °C,\n";
    payload += "  numberConcentrationPm0p5: " + String(numberConcentrationPm0p5) + " /cm3,\n";
    payload += "  numberConcentrationPm1p0: " + String(numberConcentrationPm1p0) + " /cm3,\n";
    payload += "  numberConcentrationPm2p5: " + String(numberConcentrationPm2p5) + " /cm3,\n";
    payload += "  numberConcentrationPm4p0: " + String(numberConcentrationPm4p0) + " /cm3,\n";
    payload += "  numberConcentrationPm10p0: " + String(numberConcentrationPm10p0) + " /cm3\n";
    payload += "}";
      

    // Ausgabe der Sensordaten über die serielle Schnittstelle
    Serial.println(payload);

    // Wartezeit zwischen den Messungen
    delay(Messintervall);
  
}