#include <SPI.h>
#include "display.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "pattern_parser.h"
#include "status_icons.h"

// Display states
enum DisplayState {
  STATE_LOADING,
  STATE_WIFI_CONNECTING,
  STATE_WIFI_ERROR,
  STATE_WIFI_CONNECTED,
  STATE_MQTT_CONNECTING,
  STATE_MQTT_ERROR,
  STATE_MQTT_READY,
  STATE_PATTERN_DISPLAY
};

static DisplayState currentState = STATE_LOADING;
static unsigned long stateStartTime = 0;
static bool wifiConnectedShown = false;

// MQTT message callback
void onMqttMessage(char* topic, byte* payload, unsigned int length)
{
  // Convert payload to string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  // Parse and handle the message
  parseAndDisplay(message);
}

void setup()
{
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nESP8266 Matrix Display Starting...");

  // Initialize SPI and display
  pinMode(CE, OUTPUT);
  SPI.begin();
  Serial.println("Display initialized");

  // Set loading pattern and start timer
  displayLoading();
  currentState = STATE_LOADING;
  stateStartTime = millis();

  // Initialize WiFi
  wifiInit();

  // Initialize MQTT
  mqttInit(onMqttMessage);

  Serial.println("Setup complete!");
  Serial.println("\nMQTT Message Format:");
  Serial.println("  Send 48 hex characters (RGB pattern data)");
  Serial.println("  Format: RRRRRRRRRRRRRRRRGGGGGGGGGGGGGGGGBBBBBBBBBBBBBBBB");
  Serial.println("  Example (red square): FFFFFFFFFFFFFFFF00000000000000000000000000000000");
  Serial.println("\nStatus Icons:");
  Serial.println("  White O   - Loading/startup (2s)");
  Serial.println("  Cyan ...  - WiFi connecting attempt");
  Serial.println("  Red X     - WiFi connection error (2s after each attempt)");
  Serial.println("  Yellow ✓  - WiFi connected (2s)");
  Serial.println("  Yellow    - MQTT connecting");
  Serial.println("  Blue X    - MQTT connection error (5s after 3 failed attempts)");
  Serial.println("  Green ✓   - MQTT ready, waiting for data");
}

void loop()
{
  // Always refresh the current pattern
  displayRefresh();

  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - stateStartTime;

  // State machine for display management
  switch (currentState) {
    case STATE_LOADING:
      // Show loading for 2 seconds, then transition to WiFi connecting
      if (elapsedTime >= 2000) {
        displayWifiConnecting();
        currentState = STATE_WIFI_CONNECTING;
        stateStartTime = currentTime;
        wifiConnect();  // Start WiFi connection
      }
      break;

    case STATE_WIFI_CONNECTING:
      // Check WiFi status
      if (wifiIsConnected()) {
        // WiFi connected - show yellow checkmark
        displayWifiConnected();
        currentState = STATE_WIFI_CONNECTED;
        stateStartTime = currentTime;
        wifiConnectedShown = true;
      } else if (elapsedTime >= 10000) {
        // Connection attempt timeout - show error
        displayWifiError();
        currentState = STATE_WIFI_ERROR;
        stateStartTime = currentTime;
      }
      break;

    case STATE_WIFI_ERROR:
      // Show error for 2 seconds, then retry
      if (elapsedTime >= 2000) {
        displayWifiConnecting();
        currentState = STATE_WIFI_CONNECTING;
        stateStartTime = currentTime;
        wifiConnect();  // Retry WiFi connection
      }
      break;

    case STATE_WIFI_CONNECTED:
      // Show WiFi connected for 2 seconds, then check MQTT
      if (elapsedTime >= 2000) {
        displayMqttConnecting();
        currentState = STATE_MQTT_CONNECTING;
        stateStartTime = currentTime;
      }
      break;

    case STATE_MQTT_CONNECTING:
      // Check WiFi first
      if (!wifiIsConnected()) {
        displayWifiConnecting();
        currentState = STATE_WIFI_CONNECTING;
        stateStartTime = currentTime;
        wifiConnectedShown = false;
        wifiConnect();
        break;
      }

      // Check MQTT status
      if (mqttConnectionFailed()) {
        // MQTT failed after 3 attempts - show error
        displayMqttError();
        currentState = STATE_MQTT_ERROR;
        stateStartTime = currentTime;
      } else if (mqttIsConnected()) {
        // MQTT connected - show ready icon
        displayMqttReady();
        currentState = STATE_MQTT_READY;
        stateStartTime = currentTime;
      } else {
        // Still trying to connect
        mqttLoop();
      }
      break;

    case STATE_MQTT_ERROR:
      // Show error for 5 seconds, then retry
      if (elapsedTime >= 5000) {
        mqttResetFailureState();
        displayMqttConnecting();
        currentState = STATE_MQTT_CONNECTING;
        stateStartTime = currentTime;
      }
      break;

    case STATE_MQTT_READY:
      // Check WiFi first
      if (!wifiIsConnected()) {
        displayWifiConnecting();
        currentState = STATE_WIFI_CONNECTING;
        stateStartTime = currentTime;
        wifiConnectedShown = false;
        wifiConnect();
        break;
      }

      // Check MQTT connection
      if (!mqttIsConnected()) {
        displayMqttConnecting();
        currentState = STATE_MQTT_CONNECTING;
        stateStartTime = currentTime;
      } else if (hasPatternData()) {
        // Pattern data received - switch to display mode
        updateDisplay();
        currentState = STATE_PATTERN_DISPLAY;
        stateStartTime = currentTime;
      }
      mqttLoop();
      break;

    case STATE_PATTERN_DISPLAY:
      // Check WiFi first
      if (!wifiIsConnected()) {
        displayWifiConnecting();
        currentState = STATE_WIFI_CONNECTING;
        stateStartTime = currentTime;
        wifiConnectedShown = false;
        wifiConnect();
        break;
      }

      // Check MQTT connection
      if (!mqttIsConnected()) {
        displayMqttConnecting();
        currentState = STATE_MQTT_CONNECTING;
        stateStartTime = currentTime;
      }
      // Push latest pattern data to display (handles MQTT updates)
      updateDisplay();
      mqttLoop();
      break;
  }

  // Small delay to prevent tight loop
  delay(5);
}