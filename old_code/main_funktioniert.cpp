/*
 * Copyright (c) 2024/2025 Tobias Guggenberger - software@guggenberger.me
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Ethernet.h>
#include <PubSubClient.h> // mqtt
#include <SPI.h> // Seriell
//#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//************************************************************************** LAN Network definieren 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x02 };
IPAddress ip(10, 110, 0, 7); //comment this line if you are using DHCP

//IPAddress subnet(255, 255, 0, 0); // Subnet Mask

IPAddress mqtt_server(10, 110, 0, 3);  // IP-Adresse des MQTT Brokers im lokalen Netzwerk

EthernetClient ethClient;
PubSubClient client(ethClient);

//************************************************************************** Variablen
char stgFromFloat[10];
char msgToPublish[60];
char textTOtopic[60];

//************************************************************************** WIRE Bus
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


//************************************************************************** Temp. Sensor ds18b20 HEX zuweisen
/*
DS18B20 Anschluss
rt  + 5V
sw  GND
ge  4,7 kOhm gegen rt (Pin 2 Uno)

*/
DeviceAddress temp_sensor_1    = { 0x28, 0x61, 0x64, 0x0A, 0xFD, 0x69, 0x04, 0xEB }; 
const char* topic_sensor_1     = "Heizung/Holz/Sensor1";

DeviceAddress temp_sensor_2    = { 0x28, 0x61, 0x64, 0x0A, 0xFD, 0x7D, 0x61, 0x66 }; 
const char* topic_sensor_2     = "Heizung/Holz/Sensor2";

DeviceAddress temp_sensor_3    = { 0x28, 0x61, 0x64, 0x0A, 0xF0, 0x1D, 0xFA, 0x1D }; 
const char* topic_sensor_3     = "Heizung/Holz/Sensor3";


//************************************************************************** Funktionsprototypen
void loop                       ();
void setup                      ();
void reconnect                  ();
void callback(char* topic, byte* payload, unsigned int length);
void mqtt_reconnect_intervall   ();
void temp_messen                ();


//************************************************************************** Intervalle
/* Beispiel
unsigned long previousMillis_BEISPIEL = 0; // 
unsigned long interval_BEISPIEL = 800; 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ BEISPIEL
  if (millis() - previousMillis_BEISPIEL > interval_BEISPIEL) {
      previousMillis_BEISPIEL = millis(); 
      FUNKTION();
    }  
*/
unsigned long previousMillis_mqtt_reconnect = 0; // 
unsigned long interval_mqtt_reconnect = 200; 


unsigned long previousMillis_temp_messen = 0; // Temperatur messen aufrufen
unsigned long interval_temp_messen = 5000; 

//************************************************************************** SETUP
void setup() {
  Serial.begin(115200);

// ------------------------------------------------------------------------- Ethernet starten
  Ethernet.begin(mac, ip);
  // Pause Netzwerk Antwort
  delay(1500);  

///////////////////////////////////////////////////////////////////////////  MQTT Broker init
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


}

//************************************************************************** mqtt - reconnect
void reconnect() {
  while (!client.connected()) {
    Serial.print("Verbindung zum MQTT-Server aufbauen...");
    if (client.connect("GIMA", "hitesh", "RO9UZ7wANCXzmy")) {
      Serial.println("verbunden");
      //client.subscribe("Werktor/K7");
    } else {
      Serial.print("Fehler, rc=");
      Serial.print(client.state());
      Serial.println(" erneut versuchen in 5 Sekunden");
      delay(5000);
    }
  }
}

//************************************************************************** mqtt - callback
void callback(char* topic, byte* payload, unsigned int length) {

 Serial.print("Nachricht empfangen [");
  Serial.print(topic);
  Serial.print("]: ");
  
  // Payload in einen String umwandeln
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println(message);

/*
// -------------------------------------------------------- Beispiel Topic Auswertung
// -------------------------------------------------------- Wird nur im Relaisbetrieb benötigt
  if (String(topic) == "Werktor/K3") {
    if (message == "on") {
        Serial.println("Relais K3 -> AN");
        pcf8574.digitalWrite(P3, !HIGH);
    } 
    else if (message == "off") {
        Serial.println("Relais K3 -> AUS");
        pcf8574.digitalWrite(P3, !LOW);
    } 
    else { }} else { }

*/

}



//************************************************************************** Temperatur auslesen
void temp_messen() {

sensors.requestTemperatures();

////////////////////////////////////////////////////////// Sensor 1
  int currentTemp1 = sensors.getTempC(temp_sensor_1);
  dtostrf(currentTemp1, 4, 2, stgFromFloat);
 Serial.println(currentTemp1);
   if ((currentTemp1 == -127)||(currentTemp1 == 85))  { 
     } 
    else 
        {   
  sprintf(msgToPublish, "%s", stgFromFloat);
  sprintf(textTOtopic, "%s", topic_sensor_1);
  client.publish(textTOtopic, msgToPublish);
 }


 ////////////////////////////////////////////////////////// Sensor 2
  int currentTemp2 = sensors.getTempC(temp_sensor_2);
  dtostrf(currentTemp2, 4, 2, stgFromFloat);
 Serial.println(currentTemp2);
   if ((currentTemp2 == -127)||(currentTemp2 == 85))  { 
     } 
    else 
        {   
  sprintf(msgToPublish, "%s", stgFromFloat);
  sprintf(textTOtopic, "%s", topic_sensor_2);
  client.publish(textTOtopic, msgToPublish);
 }


 ////////////////////////////////////////////////////////// Sensor 3
  int currentTemp3 = sensors.getTempC(temp_sensor_3);
  dtostrf(currentTemp3, 4, 2, stgFromFloat);
 Serial.println(currentTemp1);
   if ((currentTemp3 == -127)||(currentTemp3 == 85))  { 
     } 
    else 
        {   
  sprintf(msgToPublish, "%s", stgFromFloat);
  sprintf(textTOtopic, "%s", topic_sensor_3);
  client.publish(textTOtopic, msgToPublish);
 }


}


//************************************************************************** mqtt_reconnect_intervall 
void mqtt_reconnect_intervall() {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
}

//************************************************************************** LOOP
void loop() {


// MQTT Abfrage
    if (millis() - previousMillis_mqtt_reconnect > interval_mqtt_reconnect) {
      previousMillis_mqtt_reconnect = millis(); 
      mqtt_reconnect_intervall();
    }  

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Temperatur messen
  if (millis() - previousMillis_temp_messen > interval_temp_messen) {
      previousMillis_temp_messen= millis(); 
      // Prüfen der Panelenspannung
      //Serial.println("Temperatur messen");
      temp_messen();
    }



}