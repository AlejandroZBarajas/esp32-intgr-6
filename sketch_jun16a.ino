#include <WiFi.h>
#include <HTTPClient.h>

#define SENSOR_PIN 15  

const char* ssid = "arquitest";
const char* password = "lapiola3";

const char* serverStart = "http://10.203.103.233:8080/webcam/start";
const char* serverStop = "http://10.203.103.233:8080/webcam/stop";

bool puertaAbierta = false;
int ultimoEstadoSensor = 1;  

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando al WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");
}

void loop() {
  int valorActual = digitalRead(SENSOR_PIN);

  // Detecta transición de 1 a 0
  if (ultimoEstadoSensor == 1 && valorActual == 0) {
    if (!puertaAbierta) {
      Serial.println("Puerta ABRIENDO...");
      puertaAbierta = true;
      enviarPOST(serverStart);
    } else {
      Serial.println("Puerta CERRANDO...");
      puertaAbierta = false;
      enviarPOST(serverStop);
    }
    delay(500);  // Evita rebotes
  }

  ultimoEstadoSensor = valorActual;
  delay(20);
}

void enviarPOST(const char* url) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpCode = http.POST("");  // POST sin body
    Serial.printf("POST a %s - Código: %d\n", url, httpCode);
    http.end();
  } else {
    Serial.println("No conectado a WiFi");
  }
}
