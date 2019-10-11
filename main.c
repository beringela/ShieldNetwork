#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

const char* ssid = "TP-LINK_A8FBE7";
const char* password = "C1A8FBE7";

void setup() {
  Serial.begin(115200);
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());

  HTTPClient http;
  http.begin("https://jsonplaceholder.typicode.com/posts?userId=1");
    
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  String payload = http.getString();
  Serial.println(httpCode);
  Serial.println(payload);
  
  http.end();
}

void loop() {

}
