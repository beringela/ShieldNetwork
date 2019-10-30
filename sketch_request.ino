#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

//150.162.57.168/book?deviceKey=CCCCCCCCCCCCCCCCCCCCCC&qtd=50
//150.162.57.168/confirm?deviceKey=CCCCCCCCCCCCCCCCCCCCCC&qtd=50

const char* deviceKey;
int estoque = 500;
int reserva = 0;
int producao = 0;

bool haveMaterial = true;

ESP8266WebServer server(80);

void handleRoot();
void handleNotFound();

void setup(void){
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  Serial.print("Connecting");
  WiFi.begin("TP-LINK_A8FBE7", "C1A8FBE7");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println('\n');
  
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());     

  server.on("/book", HTTP_GET, handleBook);
  server.on("/confirm", HTTP_GET, handleConfirm);
  server.on("/insufficientStock", handleInsufficientStock);
  server.onNotFound(handleNotFound);

  server.begin();     
  Serial.println("HTTP server started");
  
  HTTPClient http;
  char *request = (char *)malloc(110 * sizeof(char));
  sprintf(request, "http://sc-central.inf.ufsc.br:5000/registerdevice?ip=%d.%d.%d.%d&port=80&type=product1&params={\"price\":1}", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  http.begin(request);
  Serial.println("Solicitando em " + (String)request);
  free(request);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  String payload = http.getString();
  Serial.println("Resposta: " + (String)httpCode);
  http.end();
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(3);
  DynamicJsonDocument dev(capacity);
  deserializeJson(dev, payload);
  char* aux = (char*)malloc(22 * sizeof(char));
  strncpy(aux, dev["deviceKey"], 22);
  deviceKey = aux;
  aux = NULL;
}

void loop(void){
  server.handleClient();
}

void handleBook() {
  const char* devAuth = server.arg("deviceKey").c_str();
  if (strcmp(deviceKey, devAuth) != 0) {
    server.send(200, "text/plain; charset=UTF-8", "Ignorando requisição, chave do dispositivo incorreta!");
  } else {
    int qtd = atoi(server.arg("qtd").c_str());
    if (qtd <= estoque) {
      estoque -= qtd;
      reserva += qtd;
      server.send(200, "application/json", "{\"status\":\"Ok!\"}");
    } else {
      server.sendHeader("Location","/insufficientStock");
      server.send(303);
    }
  }
}

void handleConfirm() {
  const char* devAuth = server.arg("deviceKey").c_str();
  if (strcmp(deviceKey, devAuth) != 0) {
    server.send(200, "text/plain; charset=UTF-8", "Ignorando requisição, chave do dispositivo incorreta!");
  } else {
    int qtd = atoi(server.arg("qtd").c_str());
    reserva -= qtd;
    producao += qtd;
    if (!haveMaterial) {
      char *output = (char *)malloc(71 * sizeof(char));
      sprintf(output, "Confirmado! Solicitando matéria-prima...\n\nestoque: %d\nreserva: %d\nproducao: %d", estoque, reserva, producao);
      server.send(200, "text/plain; charset=UTF-8", output);
      free(output);
      HTTPClient http;
      http.begin("150.162.57.181/book?deviceKey=CCCCCCCCCCCCCCCCCCCCCC&qtd=50");
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.GET();
      String payload = http.getString();
      Serial.println("Resposta: " + (String)httpCode);
      http.end();
    } else {
      char *output = (char *)malloc(71 * sizeof(char));
      sprintf(output, "Confirmado! Produção iniciada...\n\nestoque: %d\nreserva: %d\nproducao: %d", estoque, reserva, producao);
      server.send(200, "text/plain; charset=UTF-8", output);
      free(output);
    }
  }
}

void handleInsufficientStock() {
  server.send(200, "application/json", "{\"status\":\"Estoque insuficiente\"}");
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}
