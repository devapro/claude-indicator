#include "display.h"
#include <SPI.h>

// Display control variables
uint8_t data[4] = {0x00, 0x0, 0x0, 0x0};
int CE = D10;

// Current pattern storage
static uint8_t storedPattern[3][8];

void displaySetPattern(uint8_t pattern[3][8])
{
  // Copy pattern to stored pattern
  for (int c = 0; c < 3; c++) {
    for (int j = 0; j < 8; j++) {
      storedPattern[c][j] = pattern[c][j];
    }
  }
}

void displayRefresh()
{
  int j;
  int x = 666;

  for (byte c = 0; c < 3; c++) { // perform red, blue and then green scans
    for (j = 0; j < 8; j++)
    {
      data[0] = (c == 0 ? ~storedPattern[c][j] : 0xFF);  // color red
      data[2] = (c == 1 ? ~storedPattern[c][j] : 0xFF);  // color blue
      data[1] = (c == 2 ? ~storedPattern[c][j] : 0xFF);  // color green
      data[3] = 0x01 << j;                                // display the data on matrix
      digitalWrite(CE, LOW);                              // when CE is low, it begins to receive data
      SPI.transfer(data[0]);                              // transfer data[0] to the matrix (red)
      SPI.transfer(data[2]);                              // transfer data[2] to the matrix (green)
      SPI.transfer(data[1]);                              // transfer data[1] to the matrix (blue)
      SPI.transfer(data[3]);                              // transfer data[3] to the matrix (scanning and display)
      digitalWrite(CE, HIGH);                             // when CE is High, matrix begins to display
      delayMicroseconds(x);                               // delay to keep LED lit
    }
  }
}

void displayPattern(uint8_t pattern[3][8])
{
  displaySetPattern(pattern);
  displayRefresh();
}
