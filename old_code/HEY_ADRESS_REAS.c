#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>   
#include <OneWire.h>
#include <DallasTemperature.h>

/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                             ();
void findSensors              ();

/////////////////////////////////////////////////////////////////////////// DS18b20
#define SENSOR_PIN  D4 // ESP32 Pin OneWire
float tempC; // temperature in Celsius
#define TEMPERATURE_PRECISION 12

OneWire oneWire(SENSOR_PIN);
DallasTemperature DS18B20(&oneWire);

DeviceAddress tempDeviceAddress; // Verzeichniss zum Speichern von Sensor Adressen
int numberOfDevices; // Anzahl der gefundenen Sensoren
 
void findSensors()
{
  byte address[8];                                 // Store 8 ROM bytes of sensor
  byte noOfSensor = 0, i = 0, ok = 0, flag = 0;    // Diverse Hilfsvariablen
  Serial.println("DS18xx 1-Wire Temperatursensor Scanner");
  Serial.println();
  Serial.println("Suche...");
  Serial.println();
  /***  Das erste Byte enthält den Familiencode der hier ausgewertet wird  ***/
  while (oneWire.search(address))
  {
    Serial.println("-----------------------------------------------");
    Serial.print(noOfSensor += 1);
    Serial.print(". ");
    switch (address[0]) {
      case 0x10:
        Serial.println("Sensor ist ein DS18S20 oder aelterer DS1820: ");
        flag = true;                               //
        break;
      case 0x28:
        Serial.println("Sensor ist ein DS18B20: ");
        flag = true;                               //
        break;
      case 0x22:
        Serial.println("Sensor ist ein DS1822: ");
        flag = true;                               //
        break;
      default:
        Serial.println("Keinen gueltigen DS18xx Sensor gefunden!");
        return;
    }
    /***  Zeige alle 64 Adressbits sofern flag = 1 (true) ist, also alles OK  ***/
    if (flag == true)
    {
      if (OneWire::crc8(address, 7) != address[7])
      {
        Serial.println("CRC Pruefung nicht bestanden! Fehler!");
      }
      else
      {
        /***  Da bis hierhin alles Okay ist, zeige nun alle 8 Bytes  ***/
        Serial.println("HEx Ausgabe der Adresse:");
        for (i = 0; i < 8; i++)
        {
          Serial.print("0x");
          if (address[i] <= 0xF)                   // Führende 0 ggf. ergänzen
          {
            Serial.print("0");
          }
          Serial.print(address[i], HEX);           // Darstellung Hexadezimal
          if (i < 7)
          {
            Serial.print(", ");
          }
        }
        Serial.println("");
        Serial.println("Adresse aus allen 8 Bytes");
        /***  Da bis hierhin alles Okay ist, zeige nun alle 8 Bytes  ***/
        for (i = 0; i < 8; i++)
        {

          Serial.print(address[i]);           // Darstellung Hexadezimal
          if (i < 7)
          {
            Serial.print(" - ");
          }
        }
        Serial.println("");


        ok = true;                                 // ok = 1
      }
    }                                              // Ende der if Schleife
  }                                                // Ende der while Schleife
  if (ok == false)                                 // ok=0, no valid sensor found
  {
    Serial.println("Keinen gueltigen DS18xx Sensor gefunden!");
  }
  Serial.println();
  Serial.println("Suche abgeschlossen! Druecke Reset Taster fuer Neustart.");
  Serial.println();
}
 
void loop(void)
{
   // Aufruf der Funktion sensors.requestTemperatures()
   // Dadurch werden alle werte abgefragt.
   Serial.print("Abfrage der Temperatur... ");
   DS18B20.requestTemperatures();
   Serial.println("DONE");
 
   // Ausgabe der Daten für jeden Sensor
   for(int i=0 ;i<numberOfDevices; i++) {
      float tempC = DS18B20.getTempCByIndex(i);
      Serial.print("Sensor ");
      Serial.print(i, DEC);
      Serial.print(" hat grad Celsius: ");
      Serial.println(tempC);
   }
   delay(3000);
}

void setup(void)
{
   Serial.begin(115200);
   Serial.println("Abfrage mehrerer Dallas Temperatur Sensoren");
   Serial.println("-------------------------------------------");
 
findSensors(); 


   // Suche der Sensoren
   Serial.println("Suche Temperatur Sensoren...");
   DS18B20.begin();
   numberOfDevices = DS18B20.getDeviceCount();
 
   Serial.print("Habe ");
   Serial.print(numberOfDevices, DEC);
   Serial.println(" Sensoren gefunden.");
 
   // Setzen der Genauigkeit
   for(int i=0 ;i<numberOfDevices; i++) {
      if(DS18B20.getAddress(tempDeviceAddress, i)) {
         DS18B20.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
        /*
          Serial.println("Get Adress");
         if (tempDeviceAddress[i] < 16) Serial.print("0");
          Serial.print(tempDeviceAddress[i], HEX);
          */
         Serial.print("Sensor ");
         Serial.print(i);
         Serial.print(" hat eine genauigkeit von ");
         Serial.println(DS18B20.getResolution(tempDeviceAddress), DEC);
      }
   }
   Serial.println("");

   delay(250000);
 
}