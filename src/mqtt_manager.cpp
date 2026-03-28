#include "mqtt_manager.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

static WiFiClient espClient;
static PubSubClient mqttClient(espClient);
static MqttMessageCallback messageCallback = nullptr;
static unsigned long lastReconnectAttempt = 0;
static bool lastConnectionFailed = false;
static int connectionAttempts = 0;

// Internal callback wrapper
void internalMqttCallback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("MQTT message received on topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (messageCallback) {
    messageCallback(topic, payload, length);
  }
}

void mqttInit(MqttMessageCallback callback)
{
  messageCallback = callback;
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(internalMqttCallback);
}

bool mqttConnect()
{
  Serial.print("Connecting to MQTT broker: ");
  Serial.println(MQTT_SERVER);

  connectionAttempts++;
  String clientId = String(MQTT_CLIENT_ID) + "_" + String(ESP.getChipId());

  bool connected;
  if (strlen(MQTT_USER) > 0) {
    connected = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
  } else {
    connected = mqttClient.connect(clientId.c_str());
  }

  if (connected) {
    Serial.println("MQTT connected!");
    lastConnectionFailed = false;
    connectionAttempts = 0;

    // Subscribe to display topic
    if (mqttClient.subscribe(MQTT_TOPIC_DISPLAY)) {
      Serial.print("Subscribed to topic: ");
      Serial.println(MQTT_TOPIC_DISPLAY);
    }

    // Publish status
    mqttPublish(MQTT_TOPIC_STATUS, "online");

    return true;
  } else {
    Serial.print("MQTT connection failed, rc=");
    Serial.println(mqttClient.state());

    // Mark as failed after 3 attempts
    if (connectionAttempts >= 3) {
      lastConnectionFailed = true;
    }

    return false;
  }
}

bool mqttIsConnected()
{
  return mqttClient.connected();
}

void mqttLoop()
{
  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > MQTT_RECONNECT_DELAY) {
      lastReconnectAttempt = now;
      mqttReconnect();
    }
  } else {
    mqttClient.loop();
  }
}

bool mqttPublish(const char* topic, const char* message)
{
  if (!mqttClient.connected()) {
    return false;
  }
  return mqttClient.publish(topic, message);
}

bool mqttSubscribe(const char* topic)
{
  if (!mqttClient.connected()) {
    return false;
  }
  return mqttClient.subscribe(topic);
}

void mqttReconnect()
{
  if (WiFi.status() != WL_CONNECTED) {
    lastConnectionFailed = false;
    connectionAttempts = 0;
    return;
  }

  if (!mqttClient.connected()) {
    Serial.println("Attempting MQTT reconnection...");
    mqttConnect();
  }
}

bool mqttConnectionFailed()
{
  return lastConnectionFailed;
}

void mqttResetFailureState()
{
  lastConnectionFailed = false;
  connectionAttempts = 0;
}
