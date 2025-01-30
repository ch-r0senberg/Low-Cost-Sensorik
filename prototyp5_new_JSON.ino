#include <ETH.h>
#include <WiFiClient.h>
#include <PubSubClient.h> 
#include <string.h>
#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <Wire.h>



//GPIO und Messintervall
const int Messintervall = 10000; 
int sda_pin = 4;  
int scl_pin = 5;


// MQTT Broker
IPAddress server(130, 149, 137, 59);
const char *topic = "PTZ/Batterielabor/AirQuality/Sensor1";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;


//Netzwerk TU
IPAddress local_IP(130, 149, 137, 180);
IPAddress gateway(130, 149, 137, 1);     
IPAddress subnet(255, 255, 255, 0);      
IPAddress dns1(130, 149, 137, 19);            



// Variablen für die Sensordaten
float massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0;
float numberConcentrationPm0p5, numberConcentrationPm1p0, numberConcentrationPm2p5, numberConcentrationPm4p0, numberConcentrationPm10p0;
float ambientHumidity, ambientTemperature, vocIndex, noxIndex;
float typicalParticleSize;


//Objekte für Sensor und MQTT
SensirionI2CSen5x sen5x;

WiFiClient ethClient; 
PubSubClient client(ethClient);


void setup() {

    Serial.begin(115200);

    if (ETH.begin()) {
        if (!ETH.config(local_IP, gateway, subnet, dns1)) {
            Serial.println("Fehler bei der statischen IP-Konfiguration");
            while (true); // Stoppen, wenn Konfiguration fehlschlägt
        } else {
            Serial.print("Statische IP-Adresse: ");
            Serial.println(ETH.localIP());
        }
    } else {
        Serial.println("Ethernet-Initialisierung fehlgeschlagen");
    }
  

    client.setServer(server, mqtt_port);
    client.setBufferSize(512);
    connectToMQTT();


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
    if (!client.connected()) {
        connectToMQTT();
    }
    
    client.loop();

    sen5x.readMeasuredValues(massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex, noxIndex);
    sen5x.readMeasuredPmValues(massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, numberConcentrationPm0p5, numberConcentrationPm1p0, numberConcentrationPm2p5, numberConcentrationPm4p0, numberConcentrationPm10p0, typicalParticleSize);
    
    String payload = "{";
    payload += "\"Humidity\": " + String(ambientHumidity) + " %,\n";
    payload += "\"Temperature\": " + String(ambientTemperature) + " °C,\n";
    payload += "\"numberConcentrationPm0p5\": " + String(numberConcentrationPm0p5) + " /cm3,\n";
    payload += "\"numberConcentrationPm1p0\": " + String(numberConcentrationPm1p0) + " /cm3,\n";
    payload += "\"numberConcentrationPm2p5\": " + String(numberConcentrationPm2p5) + " /cm3,\n";
    payload += "\"numberConcentrationPm4p0\": " + String(numberConcentrationPm4p0) + " /cm3,\n";
    payload += "\"numberConcentrationPm10p0\": " + String(numberConcentrationPm10p0) + " /cm3\n";
    payload += "}";
      

    static unsigned long lastPublish = 0;
    unsigned long now = millis();
    if (now - lastPublish > Messintervall) { 
        
        Serial.println(payload);
        client.publish(topic, payload.c_str());
        lastPublish = now;

    }
}


void connectToMQTT() {
    while (!client.connected()) {

        String client_id = "esp32-client-";
        client_id += String(ETH.macAddress());


        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("MQTT Verbindung erfolgreich");
        } else {
            Serial.println("MQTT Verbindung fehlgeschlagen");
            Serial.println(client.state());
            delay(2000);
    }
  }
}
