#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

// Initialize WiFi connection
void wifiInit();

// Connect to WiFi network
bool wifiConnect();

// Check if WiFi is connected
bool wifiIsConnected();

// Get WiFi status as string
String wifiGetStatus();

#endif
