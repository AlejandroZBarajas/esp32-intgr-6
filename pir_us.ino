#include <WiFi.h>
#include <HTTPClient.h>

// Pines
const int PIR_PIN = 13;
const int TRIG_PIN = 12;
const int ECHO_PIN = 14;

// WiFi
const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";

// Servidor al que enviar el POST
const char* serverUrl = "http://tuservidor.com/endpoint";

// Variables globales
bool pirState = false;
bool timerRunning = false;
unsigned long startTime = 0;
unsigned long elapsedTime = 0;

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  pirState = digitalRead(PIR_PIN);
  float distance = readUltrasonic();

  if (pirState) {
    if (!timerRunning && distance < 90) {
      // Iniciar cronómetro
      startTime = millis();
      timerRunning = true;
      Serial.println("Cronómetro iniciado.");
    }
    else if (timerRunning && distance > 90) {
      // Detener cronómetro
      elapsedTime = millis() - startTime;
      timerRunning = false;
      Serial.print("Cronómetro detenido. Tiempo transcurrido: ");
      Serial.print(elapsedTime / 1000.0);
      Serial.println(" segundos.");

      sendElapsedTime(elapsedTime / 1000.0); // Enviar en segundos
    }

    delay(2000); // evitar múltiples activaciones
  }
}

float readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float distance = duration * 0.0343 / 2;
  return distance;
}

void sendElapsedTime(float seconds) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"time\": " + String(seconds, 2) + "}";

    int httpResponseCode = http.POST(payload);

    Serial.print("POST enviado. Código respuesta: ");
    Serial.println(httpResponseCode);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Respuesta del servidor: " + response);
    } else {
      Serial.println("Error en la petición POST.");
    }

    http.end();
  } else {
    Serial.println("WiFi no conectado.");
  }
}

