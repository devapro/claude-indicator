#include "pattern_parser.h"
#include "display.h"

// Current pattern storage
static uint8_t currentPattern[3][8];
static bool patternReceived = false;

void parseAndDisplay(const char* message)
{
  String msg = String(message);
  msg.trim();

  Serial.print("Received pattern data: ");
  Serial.println(msg);

  // Expected format: 48 hex characters (8 bytes each for R, G, B channels)
  // Example: FFFFFFFFFFFF0000000000000000000000000000000000000000000000000000
  // This is: 8 bytes red (all FF), 8 bytes green (all 00), 8 bytes blue (all 00) = red square

  if (msg.length() < 48) {
    Serial.println("Invalid pattern data: need 48 hex characters (R:16, G:16, B:16)");
    return;
  }

  // Parse red channel (first 16 hex chars = 8 bytes)
  for (int i = 0; i < 8; i++) {
    String byteStr = msg.substring(i * 2, i * 2 + 2);
    currentPattern[0][i] = strtol(byteStr.c_str(), NULL, 16);
  }

  // Parse green channel (next 16 hex chars = 8 bytes)
  for (int i = 0; i < 8; i++) {
    String byteStr = msg.substring(16 + i * 2, 16 + i * 2 + 2);
    currentPattern[2][i] = strtol(byteStr.c_str(), NULL, 16);
  }

  // Parse blue channel (last 16 hex chars = 8 bytes)
  for (int i = 0; i < 8; i++) {
    String byteStr = msg.substring(32 + i * 2, 32 + i * 2 + 2);
    currentPattern[1][i] = strtol(byteStr.c_str(), NULL, 16);
  }

  patternReceived = true;
  Serial.println("Pattern loaded successfully");
}

bool hasPatternData()
{
  return patternReceived;
}

void updateDisplay()
{
  if (patternReceived) {
    displaySetPattern(currentPattern);
  }
}
