#include <string.h>
#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <Wire.h>



//GPIO und Messintervall
const int Messintervall = 10000; 
int sda_pin = 4;  
int scl_pin = 5;


// Variablen für die Sensordaten
float massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0;
float numberConcentrationPm0p5, numberConcentrationPm1p0, numberConcentrationPm2p5, numberConcentrationPm4p0, numberConcentrationPm10p0;
float ambientHumidity, ambientTemperature, vocIndex, noxIndex;
float typicalParticleSize;


//Objekte für Sensor und MQTT
SensirionI2CSen5x sen5x;



void setup() {

    Serial.begin(115200);

    Wire.begin(sda_pin, scl_pin);
    sen5x.begin(Wire);
    sen5x.deviceReset();
    
    int error = sen5x.startMeasurement();
    if (error) {
      Serial.print("Fehler beim Start der Messung: ");
      Serial.println(error);
  }


}


void loop() {
   
    sen5x.readMeasuredValues(massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex, noxIndex);
    sen5x.readMeasuredPmValues(massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, numberConcentrationPm0p5, numberConcentrationPm1p0, numberConcentrationPm2p5, numberConcentrationPm4p0, numberConcentrationPm10p0, typicalParticleSize);
    
    String payload = "{\n";
    payload += "  Humidity: " + String(ambientHumidity) + " %,\n";
    payload += "  Temperature: " + String(ambientTemperature) + " °C,\n";
    payload += "  numberConcentrationPm0p5: " + String(numberConcentrationPm0p5) + " /cm3,\n";
    payload += "  numberConcentrationPm1p0: " + String(numberConcentrationPm1p0) + " /cm3,\n";
    payload += "  numberConcentrationPm2p5: " + String(numberConcentrationPm2p5) + " /cm3,\n";
    payload += "  numberConcentrationPm4p0: " + String(numberConcentrationPm4p0) + " /cm3,\n";
    payload += "  numberConcentrationPm10p0: " + String(numberConcentrationPm10p0) + " /cm3\n";
    payload += "}";
      
    Serial.println(payload);
    delay(Messintervall);
  
}