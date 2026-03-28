#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials
#define WIFI_SSID "HUAWEI-2.4G-c3Ym"
#define WIFI_PASSWORD "GazvvR5M"

// MQTT broker settings
#define MQTT_SERVER "192.168.100.66"  // Change to your MQTT broker
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "esp8266_matrix"
#define MQTT_USER "heater"  // Leave empty if no authentication
#define MQTT_PASSWORD "heater"  // Leave empty if no authentication

// MQTT topics
#define MQTT_TOPIC_DISPLAY "matrix/display"  // Topic to receive display commands
#define MQTT_TOPIC_STATUS "matrix/status"    // Topic to publish status

// Connection settings
#define WIFI_TIMEOUT_MS 20000  // 20 seconds
#define MQTT_RECONNECT_DELAY 5000  // 5 seconds

#endif
