#ifndef PATTERN_PARSER_H
#define PATTERN_PARSER_H

#include <Arduino.h>

// Parse and display pattern from MQTT message
// Message format: 48 hex characters representing RGB data
// - First 16 chars (8 bytes): Red channel
// - Next 16 chars (8 bytes): Green channel
// - Last 16 chars (8 bytes): Blue channel
// Example: "FFFFFFFFFFFFFFFF00000000000000000000000000000000" = red square
void parseAndDisplay(const char* message);

// Check if pattern data has been received
bool hasPatternData();

// Update display based on received pattern (call in loop)
void updateDisplay();

#endif
