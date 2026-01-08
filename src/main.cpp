#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"
#include "cert.h"
#include <PubSubClient.h>

const char* MQTT_SERVER = "a5247c1ae5634398b09808b959fd47e9.s1.eu.hivemq.cloud";
const uint16_t MQTT_PORT = 8883;
const char* MQTT_TOPIC_SUB = "t/LeoTestID";

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

const char url[] = "https://jsonplaceholder.typicode.com/todos/1";
const char shellyUrl[] = "http://192.168.0.124/relay/0?turn=toggle";

void toggleShelly()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(shellyUrl);
    int httpCode = http.GET(); // GET-Request an Shelly

    Serial.print("Shelly HTTP-Code: ");
    Serial.println(httpCode);

    String payload = http.getString(); // Antwort (JSON) optional
    Serial.println("Shelly-Antwort:");
    Serial.println(payload);

    http.end();
  }
  else
  {
    Serial.println("WLAN nicht verbunden, kann Shelly nicht schalten");
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("MQTT-Nachricht auf Topic: ");
  Serial.println(topic);

  String msg;
  for (unsigned int i = 0; i < length; i++)
  {
    msg += (char)payload[i];
  }
  msg.trim();
  Serial.print("Payload: ");
  Serial.println(msg);
  toggleShelly();
}

void setupMqtt()
{
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  while (!mqttClient.connected())
  {
    Serial.print("Verbinde mit MQTT-Broker ... ");
    String clientId = "esp32-client-";
    clientId += String(WiFi.macAddress());

    // Use MQTT username/password from secrets.h
    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD))
    {
      Serial.println("verbunden");
      mqttClient.subscribe(MQTT_TOPIC_SUB);
      Serial.print("Subscribed auf: ");
      Serial.println(MQTT_TOPIC_SUB);
    }
    else
    {
      Serial.print("fehlgeschlagen, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" -> neuer Versuch in 5s");
      delay(5000);
    }
  }
}

void handleMqtt() {
  if (!mqttClient.connected()) {
    setupMqtt();   // neu verbinden, falls getrennt
  }
  mqttClient.loop();  // muss regelmäßig aufgerufen werden
}

void getSmhDevices()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WLAN nicht verbunden");
    return;
  }

  WiFiClientSecure client;
  client.setCACert(ROOT_CERT_SMARTVITAAPI);

  HTTPClient https;

  String url = "https://api.smart-vita.de/services/2.0/smh-devices";

  if (!https.begin(client, url))
  {
    Serial.println("HTTPS begin() fehlgeschlagen");
    return;
  }

  // Authorization-Header mit Bearer-Token
  String authHeader = String("Bearer ") + ACCESS_TOKEN;
  https.addHeader("Authorization", authHeader);

  int httpCode = https.GET();

  Serial.print("HTTP-Code: ");
  Serial.println(httpCode);

  if (httpCode > 0)
  {
    String payload = https.getString();
    Serial.println("Antwort von /smh-devices:");
    Serial.println(payload);
  }
  else
  {
    Serial.print("HTTPS-Fehler: ");
    Serial.println(httpCode);
  }

  https.end();
}

void request_jsonplaceholder()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClientSecure client;
    // client.setInsecure(); // *** UNSICHER, nur für Tests ***
    client.setCACert(ROOT_CERT_JSONPLACEHOLDER);

    HTTPClient https;
    if (https.begin(client, url))
    {
      int httpCode = https.GET();
      if (httpCode > 0)
      {
        Serial.print("HTTP-Code: ");
        Serial.println(httpCode);

        String payload = https.getString();
        Serial.println("Antwort:");
        Serial.println(payload);
      }
      else
      {
        Serial.print("HTTPS-Fehler: ");
        Serial.println(httpCode);
      }
      https.end();
    }
    else
    {
      Serial.println("HTTPS begin() fehlgeschlagen");
    }
  }
  else
  {
    Serial.println("WLAN nicht verbunden");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("Verbinde mit WLAN...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden!");
  Serial.println(WiFi.localIP());

  // Configure TLS for MQTT connection. If ROOT_CERT_HIVEMQ is empty,
  // fall back to insecure connection (useful for quick testing only).
  if (ROOT_CERT_HIVEMQ[0] != '\0') {
    espClient.setCACert(ROOT_CERT_HIVEMQ);
    Serial.println("MQTT: using certificate from cert.h");
  } else {
    espClient.setInsecure();
    Serial.println("MQTT: WARNING - certificate verification DISABLED (setInsecure()).");
  }

  setupMqtt();
  // pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  // Serial.println("\nTEST!");
  //  toggleShelly();
  // request_jsonplaceholder();
  // getSmhDevices();
  //  static bool state = false;
  //  state = !state;
  //  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
  handleMqtt();
  delay(10);
}