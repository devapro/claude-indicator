#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// External variables for display control
extern uint8_t data[4];
extern int CE;

// Set the current pattern to be displayed (stores it for continuous display)
void displaySetPattern(uint8_t pattern[3][8]);

// Refresh the currently stored pattern (call continuously in loop)
void displayRefresh();

// Generic function to display a pixel pattern on the matrix (stores and displays)
// pattern: 3x8 array where [0] = red, [1] = blue, [2] = green pixels
void displayPattern(uint8_t pattern[3][8]);

#endif
