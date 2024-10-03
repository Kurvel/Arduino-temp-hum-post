#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include "settings.h"
#include "DHT.h"

#define DHTPIN 2 
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

char ssid[] = SECRET_SSID;
char password[] = SECRET_PASS;

char serverAdress[] = "192.168.1.166";  
int port = 8080;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAdress, port);



void setup() {
  // WiFi Setup
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  dht.begin();

  Serial.println("Ansluter till Wifi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(500);
  }
  Serial.println("Ansluten till WiFi");
}

void loop() {
  
  delay(2000);

  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  

  
  String postData = "{\"temp\":\"" + String(t, 2) + "\",\"humidity\":\"" + String(h, 2) + "\"}";

  if (WiFi.status() != WL_CONNECTED) {
  Serial.println("WiFi connection lost. Reconnecting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Reconnected to WiFi.");
}

  bool success = false;
  int retryCount = 0;
  const int maxRetries = 3;

  while (!success && retryCount < maxRetries) {
    Serial.println("Skickar vår POST");
    client.beginRequest();
    client.post("/api/sensor");

    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", postData.length());

    client.beginBody();
    client.print(postData);
    client.endRequest();

    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    if (statusCode == 200) {
      success = true;
      Serial.println("POST request successful.");
    } else {
      retryCount++;
      Serial.println("POST request failed, retrying...");
      delay(2000); 
    }
  }

  if (!success) {
    Serial.println("Failed to send POST after retries.");
  }

  delay(900000);
}

