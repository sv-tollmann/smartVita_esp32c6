#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"
#include "cert.h"

const char *url = "https://jsonplaceholder.typicode.com/todos/1";
const char *shellyUrl = "http://192.168.0.124/relay/0?turn=toggle";

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
void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("Verbinde mit WLAN...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden!");
  Serial.println(WiFi.localIP());

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  //toggleShelly();
  static bool state = false;
  state = !state;
  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);

  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClientSecure client;
    //client.setInsecure(); // *** UNSICHER, nur für Tests ***
    client.setCACert(ROOT_CERT);

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

  delay(10000);
}