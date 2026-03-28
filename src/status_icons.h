#ifndef STATUS_ICONS_H
#define STATUS_ICONS_H

#include <Arduino.h>

// Display loading/boot pattern
void displayLoading();

// Display WiFi connecting pattern (single frame) - shown during connection attempt
void displayWifiConnectingFrame();

// Display WiFi connecting pattern (continuous) - shown during connection attempt
void displayWifiConnecting();

// Display WiFi error pattern (single frame) - shown when connection fails
void displayWifiErrorFrame();

// Display WiFi error pattern (continuous) - shown when connection fails
void displayWifiError();

// Display WiFi connected pattern (single frame) - shown when WiFi successfully connects
void displayWifiConnectedFrame();

// Display WiFi connected pattern (continuous) - shown when WiFi successfully connects
void displayWifiConnected();

// Display WiFi connected / MQTT connecting icon
void displayMqttConnecting();

// Display MQTT error pattern (single frame) - shown when MQTT connection fails
void displayMqttErrorFrame();

// Display MQTT error pattern (continuous) - shown when MQTT connection fails
void displayMqttError();

// Display MQTT ready icon (waiting for data)
void displayMqttReady();

#endif
