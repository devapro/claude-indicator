#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>

// Callback function type for received messages
typedef void (*MqttMessageCallback)(char* topic, byte* payload, unsigned int length);

// Initialize MQTT client
void mqttInit(MqttMessageCallback callback);

// Connect to MQTT broker
bool mqttConnect();

// Check if MQTT is connected
bool mqttIsConnected();

// Process MQTT loop (must be called regularly)
void mqttLoop();

// Publish a message to a topic
bool mqttPublish(const char* topic, const char* message);

// Subscribe to a topic
bool mqttSubscribe(const char* topic);

// Reconnect to MQTT broker if disconnected
void mqttReconnect();

// Check if MQTT connection failed (after multiple attempts)
bool mqttConnectionFailed();

// Reset MQTT failure state
void mqttResetFailureState();

#endif
