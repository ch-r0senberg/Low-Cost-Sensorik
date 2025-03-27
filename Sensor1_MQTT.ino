// Einbindung der notwendigen Bibliotheken
#include <ETH.h> 
#include <WiFiClient.h> 
#include <PubSubClient.h> 
#include <string.h>
#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <Wire.h>



// Definition von GPIO-Pins und Messintervall
const int Messintervall = 600000;  // Messintervall in Millisekunden (10 Minuten)
int sda_pin = 4;  // SDA-Pin auf den ESP32, benötigt für die I2C Kommunikation
int scl_pin = 5;  // SCL-Pin auf den ESP32, benötigt für die I2C Kommunikation


// MQTT Broker Daten
IPAddress server(130, 149, 137, 59);
const char *topic = "PTZ/Batterielabor/AirQuality/Sensor_1"; // MQTT-Topic zur Datenübertragung (Auf die Endung /Sensor_X achten)
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;


//Netzwerk Daten 
IPAddress local_IP(130, 149, 137, 180); // Statische IP-Adresse für den Sensor_1
IPAddress gateway(130, 149, 137, 1);     
IPAddress subnet(255, 255, 255, 0);      
IPAddress dns1(130, 149, 137, 19);            



// Variablen für die Sensordaten
float massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0;
float numberConcentrationPm0p5, numberConcentrationPm1p0, numberConcentrationPm2p5, numberConcentrationPm4p0, numberConcentrationPm10p0;
float ambientHumidity, ambientTemperature, vocIndex, noxIndex;
float typicalParticleSize;


//Objekte für Sensor und MQTT
SensirionI2CSen5x sen5x; // Sensor-Objekt

WiFiClient ethClient;  // Ethernet-Client für Netzwerkverbindungen
PubSubClient client(ethClient); // MQTT-Client-Objekt


// Setup-Funktion, wird einmal beim Start aufgerufen
void setup() {

    Serial.begin(115200); // Serielle Kommunikation starten mit 115200 Baud


    // Initialisierung der Ethernet Verbindung mit Vergabe der lokalen IP für den Sensor
    if (ETH.begin()) {
        if (!ETH.config(local_IP, gateway, subnet, dns1)) {
            Serial.println("Fehler bei der statischen IP-Konfiguration");
            // Läuft weiter aber mit DHCP, falls statische IP fehlschlägt
        } else {
            Serial.print("Statische IP-Adresse: ");  // Ausgabe der loakeln IP-Adresse im Serial Monitor -> Kontrolle, ob die Vergabe funktioniert hat
            Serial.println(ETH.localIP());
        }
    } else {
        Serial.println("Ethernet-Initialisierung fehlgeschlagen");
    }
  

    // Verbindung zum MQTT-Server herstellen
    client.setServer(server, mqtt_port);
    client.setBufferSize(512);   // Puffergröße für MQTT-Nachrichten setzen (default ist zu klein)
    connectToMQTT();             // Funktion zum Verbindungsaufbau mit dem MQTT-Broker aufrufen

    // Initialisierung des Sensors
    Wire.begin(sda_pin, scl_pin);
    sen5x.begin(Wire);
    sen5x.deviceReset();
    
    // Messung starten
    int error = sen5x.startMeasurement();
    if (error) {
      Serial.print("Fehler beim Start der Messung: ");
      Serial.println(error);
}


}

// Loop-Funktion, läuft kontinuierlich
void loop() {
    // Prüfen, ob die MQTT-Verbindung noch besteht, falls nicht, neu verbinden
    if (!client.connected()) {
        connectToMQTT();
    }
    
    client.loop(); // MQTT-Client-Loop ausführen

    // Sensordaten auslesen
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
      


    //Senden der Sensordaten an MQTT-Server, Messintervall alle 10 Minuten (je nach Festlegung Varaible Messintervall)
    static unsigned long lastPublish = 0;
    unsigned long now = millis();
    if (now - lastPublish > Messintervall) {      // Überprüfung, ob Intervall erreicht wurde
        
        Serial.println(payload);                  // Ausgabe der Sensordaten in Serial.Monitor
        client.publish(topic, payload.c_str());   // Senden der Daten an den MQTT-Server
        lastPublish = now;

    }
}

// Funktion zur Herstellung der MQTT-Verbindung
void connectToMQTT() {
    while (!client.connected()) {       // Solange nicht verbunden, versucht er zu verbinden

        String client_id = "esp32-client-";
        client_id += String(ETH.macAddress());

        // Verbindungsversuch zum MQTT-Broker
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("MQTT Verbindung erfolgreich");
        } else {
            Serial.println("MQTT Verbindung fehlgeschlagen");
            Serial.println(client.state());
            delay(2000);  // 2 Sekunden warten und erneut versuchen
    }
  }
}
