#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

// Pines
const int PIR_PIN = 13;
const int TRIG_PIN = 19;
const int ECHO_PIN = 23;

// WiFi
const char* ssid = "arquitest";
const char* password = "lapiola3";

// Servidor
const char* serverUrl = "http://10.78.48.233:8080/atracciones";

// Constante
const char* nombreConstante = "cuadro1";

// Variables
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

  // Configurar NTP (zona horaria GMT-6, ajusta si lo necesitas)
  configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  // Esperar sincronización de hora
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Esperando hora NTP...");
    delay(1000);
  }
  Serial.println("Hora sincronizada.");
}

void loop() {
  pirState = digitalRead(PIR_PIN);
  float distance = readUltrasonic();

  if (pirState) {
    if (!timerRunning && distance < 90) {
      startTime = millis();
      timerRunning = true;
      Serial.println("Cronómetro iniciado.");
    } 
    else if (timerRunning && distance > 90) {
      elapsedTime = millis() - startTime;
      timerRunning = false;

      float segundos = elapsedTime / 1000.0;
      Serial.print("Cronómetro detenido. Tiempo transcurrido: ");
      Serial.print(segundos);
      Serial.println(" segundos.");

      sendElapsedTime(segundos);
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
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error obteniendo la hora.");
    return;
  }

  char horaStr[6];   // hh:mm
  char fechaStr[11]; // yyyy-mm-dd
  strftime(horaStr, sizeof(horaStr), "%H:%M", &timeinfo);
  strftime(fechaStr, sizeof(fechaStr), "%Y-%m-%d", &timeinfo);

  int segundosEnteros = (int)seconds;  // Convertir a entero truncando decimales

  String json = "{";
  json += "\"nombre\":\"" + String(nombreConstante) + "\",";
  json += "\"tiempo\":" + String(segundosEnteros) + ",";  // Aquí usas el entero
  json += "\"hora\":\"" + String(horaStr) + "\",";
  json += "\"fecha\":\"" + String(fechaStr) + "\",";
  json += "\"enviado\":false";
  json += "}";

  Serial.println("Enviando JSON:");
  Serial.println(json);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(json);

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
