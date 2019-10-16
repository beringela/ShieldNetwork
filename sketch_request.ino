#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>


//192.168.0.38/request?codProduto=123&qtd=1000

const int estoque = 500;

ESP8266WebServer server(80);

void handleRoot();
void handleNotFound();

void setup(void){
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  Serial.print("Connecting");
  WiFi.begin("Futon", "OdamaRasenshuriken");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println('\n');
  
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());     

  if (MDNS.begin("wemos")) {     
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  server.on("/request", HTTP_GET, handleRequest);     
  server.on("/insufficientStock", handleInsufficientStock);
  server.onNotFound(handleNotFound);

  server.begin();     
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient(); 
}

void handleRequest() {
  String codProduto = server.arg("codProduto");
  int qtd = atoi(server.arg("qtd").c_str());
  if (qtd > estoque) {
    server.sendHeader("Location","/insufficientStock");
    server.send(303);
  }
}

void handleInsufficientStock() {
  server.send(200, "text/plain", "Estoque insuficiente, solicitando para o fornecedor...");
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}
