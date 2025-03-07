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
 * 
 * Temperatur Messung Trafostation
 * DHT11 Ermittlung Luftfeuchtigkeit
 */

#include <Ethernet.h>
#include <PubSubClient.h> // mqtt
#include <SPI.h> // Seriell
//#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h> // DHT11 - Luftfeuchtigkeit
#include <DHT_U.h> // DHT11 - Luftfeuchtigkeit

//************************************************************************** Variablen
char stgFromFloat[10];
char msgToPublish[60];
char textTOtopic[60];

//************************************************************************** DHT11 Sensor
#define DHTPIN 4
#define DHTTYPE    DHT11

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;
// mqtt
const char* topic_dht_temp     = "Temperatur/Trafo_GIMA_01_DHT/temp";
const char* topic_dht_humidity = "Temperatur/Trafo_GIMA_01_DHT/humidity";


//************************************************************************** LAN Network definieren 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x07 };
IPAddress ip(10, 110, 0, 12); //comment this line if you are using DHCP

//IPAddress subnet(255, 255, 0, 0); // Subnet Mask

IPAddress mqtt_server(10, 110, 0, 3);  // IP-Adresse des MQTT Brokers im lokalen Netzwerk

EthernetClient ethClient;
PubSubClient client(ethClient);

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

Steckerbelegung Sensoren
************************
L     + 5V  (braun)
N     GND   (blau)
GrGe  Datenleitung  (grge)

DHT11 Sensor
************
Pin Arduino konfigurieren
#define DHTPIN 4

*/
DeviceAddress temp_sensor_1    = { 0x28, 0xFF, 0x64, 0x1F, 0x7D, 0x80, 0xDF, 0x22 }; 
const char* topic_sensor_1     = "Temperatur/Trafo_GIMA_01/sensor1";

DeviceAddress temp_sensor_2    = { 0x28, 0xFF, 0x64, 0x1F, 0x7F, 0xCD, 0x6F, 0x4D }; 
const char* topic_sensor_2     = "Temperatur/Trafo_GIMA_01/sensor2";

DeviceAddress temp_sensor_3    = { 0x28, 0xFF, 0x64, 0x1F, 0x7C, 0x44, 0x88, 0x5E }; 
const char* topic_sensor_3     = "Temperatur/Trafo_GIMA_01/sensor3";


//************************************************************************** Funktionsprototypen
void loop                       ();
void setup                      ();
void reconnect                  ();
void callback(char* topic, byte* payload, unsigned int length);
void mqtt_reconnect_intervall   ();
void temp_messen                ();
void luftfeuchtigkeit_messen    ();
void(* resetFunc) (void) = 0;


//************************************************************************** Inter

unsigned long previousMillis_mqtt_reconnect = 0; // 
unsigned long interval_mqtt_reconnect = 500; 

unsigned long previousMillis_temp_messen = 0; // Temperatur messen aufrufen
unsigned long interval_temp_messen = 10000; 

unsigned long previousMillis_luftfeuchtigkeit_messen = 0; // Temperatur messen aufrufen
unsigned long interval_luftfeuchtigkeit_messen = 10000; 

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

///////////////////////////////////////////////////////////////////////////  DHT Sensor initialisieren
dht.begin();
sensor_t sensor;

}

//************************************************************************** mqtt - reconnect
void reconnect() {
  while (!client.connected()) {
    Serial.print("Verbindung zum MQTT-Server aufbauen...");
    if (client.connect("Temp_GIMAtestPUTZ01", "hitesh", "RO9UZ7wANCXzmy")) {
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

}


//************************************************************************** Luftfeuchtigkeit auslesen
void luftfeuchtigkeit_messen() {
 delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Fehler Temp.Messund DHT11"));
  }
  else {
      // Messdaten per mqtt senden
      dtostrf(event.temperature, 4, 2, stgFromFloat);
      sprintf(msgToPublish, "%s", stgFromFloat);
      sprintf(textTOtopic, "%s", topic_dht_temp);
      client.publish(textTOtopic, msgToPublish);
      Serial.println(event.relative_humidity); 
  }


  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Fehler Luftfeuchtigkeit .Messund DHT11"));
  }
  else {  
      // Messdaten per mqtt senden
      dtostrf(event.relative_humidity, 4, 2, stgFromFloat);
      sprintf(msgToPublish, "%s", stgFromFloat);
      sprintf(textTOtopic, "%s", topic_dht_humidity);
      client.publish(textTOtopic, msgToPublish);
      Serial.println(event.relative_humidity);  
  }
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

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ luftfeuchtigkeit_messen
  if (millis() - previousMillis_luftfeuchtigkeit_messen > interval_luftfeuchtigkeit_messen) {
      previousMillis_luftfeuchtigkeit_messen= millis(); 
      // Prüfen der Panelenspannung
      //Serial.println("Temperatur messen");
      luftfeuchtigkeit_messen();
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Arduino Reset
if ( millis()  >= 5000000) resetFunc(); // Reset alle 10 Min


}