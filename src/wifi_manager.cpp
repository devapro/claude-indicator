#include "wifi_manager.h"
#include "config.h"
#include "status_icons.h"
#include <ESP8266WiFi.h>

void wifiInit()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

bool wifiConnect()
{
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // WiFi connection is non-blocking - just initiate it
  // The main loop will check status and handle display updates

  return false;  // Return false since not connected yet
}

bool wifiIsConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

String wifiGetStatus()
{
  switch (WiFi.status()) {
    case WL_CONNECTED:
      return "Connected";
    case WL_NO_SSID_AVAIL:
      return "SSID not available";
    case WL_CONNECT_FAILED:
      return "Connection failed";
    case WL_IDLE_STATUS:
      return "Idle";
    case WL_DISCONNECTED:
      return "Disconnected";
    default:
      return "Unknown";
  }
}
