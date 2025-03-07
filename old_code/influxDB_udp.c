#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

const int sensorPin = A0;
int sensorVal;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

byte host = {192, 168, 0, 153};
unsigned int port = 8089; // local port to listen on

EthernetUDP Udp;

void setup(){
//////////////////////
// PUT YOUR SETUP CODE HERE TO RUN ONCE
//////////////////////

  Serial.begin(9600); // open serial port

  Ethernet.begin(mac, host);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found. Sorry, can’t run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println(“Ethernet cable is not connected.”);
  }
  // start UDP
  Udp.begin(port);
}

float getTemperature() {
  sensorVal = analogRead(sensorPin);
  float voltage = (sensorVal/1024.0) * 5.0;
  float temperatureC = (voltage - 0.5)*100;

  return temperatureC;
}

void loop(){
//////////////////////
// PUT YOUR MAIN CODE HERE; TO RUN REPEATEDLY
//////////////////////

  String line, temperature;
  delay(1000);
  temperature = String(getTemperature(), 2);
  Serial.println(temperature);
  line = String(“temperature value=” + temperature);
  Serial.println(line);

  Serial.println(“Sending UDP packet…”);
  Udp.beginPacket(host, port);
  Udp.print(“temperature value=”);
  Udp.print(temperature);
  Udp.endPacket();
}