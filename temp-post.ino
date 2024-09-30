#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include "settings.h"

char ssid[] = SECRET_SSID;
char password[] = SECRET_PASS;

char serverAdress[] = "192.168.1.166";  // Server IP
int port = 8080;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAdress, port);

// DHT11 Sensor code
int DHpin = 8; // input/output pin
byte dat[5];

byte read_data() {
  byte data;
  for (int i = 0; i < 8; i++) {
    if (digitalRead(DHpin) == LOW) {
      while (digitalRead(DHpin) == LOW); // wait 50us;
      delayMicroseconds(30); // Duration of high level determines whether data is 0 or 1
      if (digitalRead(DHpin) == HIGH)
        data |= (1 << (7 - i)); // High in the former, low in the post;
      while (digitalRead(DHpin) == HIGH); // Data '1', waiting for next bit
    }
  }
  return data;
}

void start_test() {
  digitalWrite(DHpin, LOW); // Pull down the bus to send the start signal;
  delay(30); // The delay is greater than 18 ms so that DHT 11 can detect the start signal;
  digitalWrite(DHpin, HIGH);
  delayMicroseconds(40); // Wait for DHT11 to respond;
  pinMode(DHpin, INPUT);
  while (digitalRead(DHpin) == HIGH);
  delayMicroseconds(80); // The DHT11 responds by pulling the bus low for 80us;
  if (digitalRead(DHpin) == LOW);
  delayMicroseconds(80); // DHT11 pulled up after the bus 80us to start sending data;
  for (int i = 0; i < 4; i++) // Receiving data, check bits are not considered;
    dat[i] = read_data();
  pinMode(DHpin, OUTPUT);
  digitalWrite(DHpin, HIGH); // After release of bus, wait for host to start next signal
}

void setup() {
  // WiFi Setup
  Serial.begin(9600);
  pinMode(DHpin, OUTPUT);

  Serial.println("Ansluter till Wifi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(500);
  }
  Serial.println("Ansluten till WiFi");
}

void loop() {
  // Read DHT11 sensor
  start_test();
  int humidityInt = dat[0]; // Integer part of humidity
  int humidityDec = dat[1]; // Decimal part of humidity
  int tempInt = dat[2];     // Integer part of temperature
  int tempDec = dat[3];     // Decimal part of temperature

  Serial.print("Current humidity = ");
  Serial.print(humidityInt);
  Serial.print('.');
  Serial.print(humidityDec);
  Serial.println('%');
  Serial.print("Current temperature = ");
  Serial.print(tempInt);
  Serial.print('.');
  Serial.print(tempDec);
  Serial.println('C');

  // Prepare JSON payload with temp and humidity
  String postData = "{\"temp\":\"" + String(tempInt) + "." + String(tempDec) + "\",\"humidity\":\"" + String(humidityInt) + "." + String(humidityDec) + "\"}";

  if (WiFi.status() != WL_CONNECTED) {
  Serial.println("WiFi connection lost. Reconnecting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Reconnected to WiFi.");
}


  // Send POST request to server
  Serial.println("Skickar vår POST");
  client.beginRequest();
  client.post("/temps/temp");

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

  delay(900000);
}

