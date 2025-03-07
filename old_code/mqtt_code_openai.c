#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Netzwerkanmeldedaten
const char* ssid = "DEIN_SSID";
const char* password = "DEIN_PASSWORT";

// MQTT-Broker
const char* mqtt_server = "test.mosquitto.org";  // Du kannst auch einen eigenen Broker verwenden

WiFiClient espClient;
PubSubClient client(espClient);

// Funktion, um eine Verbindung zum WLAN herzustellen
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Verbinde mit ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WLAN verbunden");
  Serial.println("IP-Adresse: ");
  Serial.println(WiFi.localIP());
}

// Callback-Funktion, die aufgerufen wird, wenn eine MQTT-Nachricht empfangen wird
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Nachricht empfangen [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Verbindung zum MQTT-Server herstellen
void reconnect() {
  while (!client.connected()) {
    Serial.print("Verbinde mit MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("verbunden");
      // Abonnieren des gewünschten Themas (Topic)
      client.subscribe("test/topic");
    } else {
      Serial.print("Verbindung fehlgeschlagen, rc=");
      Serial.print(client.state());
      Serial.println(" Versuche es erneut in 5 Sekunden");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Senden einer Beispielnachricht an ein bestimmtes Thema
  client.publish("test/topic", "Hallo von Arduino!");
  delay(10000);  // Nachricht alle 10 Sekunden senden
}
