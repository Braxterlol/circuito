#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

// Configuración de WiFi
const char* ssid = "FamRamos1";
const char* password = "Juarez2006";

// Configuración de MQTT
const char* mqtt_server = "44.223.219.154";  // IP o dirección del servidor RabbitMQ con plugin MQTT
const int mqtt_port = 1883;  // Puerto del servidor MQTT

// Configuración del webhook de Discord
const char* discord_webhook_url = "https://discord.com/api/webhooks/1252373821286125712/mjqwj7pK6UpOi7cSSS9Fiouzl2etaoqImDZVRHrTLntanqeCCBukafXTi3cCegYeqwlI";

// Definir los pines digitales del micrófono y del sensor PIR
const int micDigitalPin = 19;  // GPIO19 del ESP32
const int pirDigitalPin = 18;  // GPIO18 del ESP32

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Iniciar la comunicación serial para monitorear los valores
  Serial.begin(74880);
  
  // Configurar los pines del micrófono y del PIR como entradas
  pinMode(micDigitalPin, INPUT);
  pinMode(pirDigitalPin, INPUT);

  // Conectar a la red WiFi
  setup_wifi();

  // Configurar el servidor MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Mantener la conexión MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Leer el estado del pin digital del micrófono y del PIR
  int soundDetected = digitalRead(micDigitalPin);
  int motionDetected = digitalRead(pirDigitalPin);
  
  if (soundDetected == HIGH) {
    Serial.println("¡Sonido detectado!");
    sendAlertToRabbitMQ("¡Sonido detectado!");
    sendAlertToDiscord("¡Sonido detectado!");
  } 
  
  if (motionDetected == HIGH) {
    Serial.println("¡Movimiento detectado!");
    sendAlertToRabbitMQ("¡Movimiento detectado!");
    sendAlertToDiscord("¡Movimiento detectado!");
  }
  
  // Esperar un poco antes de la siguiente lectura
  delay(3000);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop hasta que se vuelva a conectar
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    // Intentar conectar
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado");
    } else {
      Serial.print("Error, rc=");
      Serial.print(client.state());
      Serial.println(" Intentar de nuevo en 5 segundos");
      // Esperar 5 segundos antes de reintentar
      delay(5000);
    }
  }
}

void sendAlertToRabbitMQ(const char* message) {
  // Publicar el mensaje en el tema "alerta.mov"
  if (client.publish("alerta.mov", message)) {
    Serial.println("Alerta enviada a RabbitMQ");
  } else {
    Serial.println("Error al enviar la alerta");
  }
}

void sendAlertToDiscord(const char* message) {
  HTTPClient http;
  String payload = "{\"content\": \"" + String(message) + "\"}";

  http.begin(discord_webhook_url);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Alerta enviada a Discord");
    Serial.println(response);
  } else {
    Serial.println("Error al enviar la alerta a Discord");
  }
  http.end();
}
