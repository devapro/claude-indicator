#include "weather.h"
#include "display.h"

// Day weather icons (8x8 patterns)

// Sunny day
static uint8_t sunny[8] = {
  0x99,  // 10011001
  0x5A,  // 01011010
  0x3C,  // 00111100
  0xFF,  // 11111111
  0xFF,  // 11111111
  0x3C,  // 00111100
  0x5A,  // 01011010
  0x99   // 10011001
};

// Partly cloudy day
static uint8_t partlyCloudy[8] = {
  0x88,  // 10001000
  0x48,  // 01001000
  0x3C,  // 00111100
  0x7E,  // 01111110
  0xFF,  // 11111111
  0xFF,  // 11111111
  0x7E,  // 01111110
  0x00   // 00000000
};

// Cloudy day
static uint8_t cloudy[8] = {
  0x00,  // 00000000
  0x38,  // 00111000
  0x7C,  // 01111100
  0xFE,  // 11111110
  0xFE,  // 11111110
  0x7C,  // 01111100
  0x00,  // 00000000
  0x00   // 00000000
};

// Rainy
static uint8_t rainy[8] = {
  0x38,  // 00111000
  0x7C,  // 01111100
  0xFE,  // 11111110
  0xFE,  // 11111110
  0x00,  // 00000000
  0x92,  // 10010010
  0x92,  // 10010010
  0x92   // 10010010
};

// Stormy (thunderstorm)
static uint8_t stormy[8] = {
  0x38,  // 00111000
  0x7C,  // 01111100
  0xFE,  // 11111110
  0x18,  // 00011000
  0x30,  // 00110000
  0x60,  // 01100000
  0x30,  // 00110000
  0x18   // 00011000
};

// Snowy
static uint8_t snowy[8] = {
  0x38,  // 00111000
  0x7C,  // 01111100
  0xFE,  // 11111110
  0x00,  // 00000000
  0xAA,  // 10101010
  0x00,  // 00000000
  0x55,  // 01010101
  0x00   // 00000000
};

// Windy
static uint8_t windy[8] = {
  0xFF,  // 11111111
  0x00,  // 00000000
  0x7E,  // 01111110
  0x00,  // 00000000
  0xFF,  // 11111111
  0x00,  // 00000000
  0x7E,  // 01111110
  0x00   // 00000000
};

// Night weather icons

// Clear night (moon and stars)
static uint8_t clearNight[8] = {
  0x84,  // 10000100
  0x1C,  // 00011100
  0x18,  // 00011000
  0x1C,  // 00011100
  0x08,  // 00001000
  0x00,  // 00000000
  0x42,  // 01000010
  0x00   // 00000000
};

// Crescent moon
static uint8_t crescentMoon[8] = {
  0x00,  // 00000000
  0x1C,  // 00011100
  0x3E,  // 00111110
  0x38,  // 00111000
  0x38,  // 00111000
  0x3E,  // 00111110
  0x1C,  // 00011100
  0x00   // 00000000
};

// Cloudy night
static uint8_t cloudyNight[8] = {
  0x0C,  // 00001100
  0x1C,  // 00011100
  0x78,  // 01111000
  0xFC,  // 11111100
  0xFE,  // 11111110
  0xFE,  // 11111110
  0x7C,  // 01111100
  0x00   // 00000000
};

// Rainy night
static uint8_t rainyNight[8] = {
  0x1C,  // 00011100
  0x7C,  // 01111100
  0xFE,  // 11111110
  0xFE,  // 11111110
  0x00,  // 00000000
  0x92,  // 10010010
  0x92,  // 10010010
  0x92   // 10010010
};

// Starry night
static uint8_t starryNight[8] = {
  0x88,  // 10001000
  0x50,  // 01010000
  0x20,  // 00100000
  0x00,  // 00000000
  0x02,  // 00000010
  0x05,  // 00000101
  0x02,  // 00000010
  0x44   // 01000100
};

// Function to create a color pattern from weather icon and color channels
static void createColorPattern(uint8_t weather[8], bool useRed, bool useGreen, bool useBlue, uint8_t output[3][8])
{
  for (int i = 0; i < 8; i++) {
    output[0][i] = useRed ? weather[i] : 0x00;    // Red channel
    output[1][i] = useBlue ? weather[i] : 0x00;   // Blue channel
    output[2][i] = useGreen ? weather[i] : 0x00;  // Green channel
  }
}

// Function to display weather icons periodically with different colors
void displayWeatherCycle()
{
  uint8_t pattern[3][8];
  static int weatherIndex = 0;
  static int colorIndex = 0;

  // Define colors (R, G, B) - different colors for day/night
  bool dayColors[4][3] = {
    {true, true, false},   // Yellow (sun)
    {false, false, true},  // Blue (sky/rain)
    {true, false, false},  // Red (sunset)
    {true, true, true}     // White (clouds)
  };

  bool nightColors[3][3] = {
    {false, false, true},  // Blue (night sky)
    {true, true, true},    // White (moon/stars)
    {true, false, true}    // Magenta (night)
  };

  // Determine if it's day or night based on index
  bool isDay = weatherIndex < 7;

  // Select weather icon based on index
  uint8_t* selectedWeather;
  switch (weatherIndex) {
    // Day weather (0-6)
    case 0:
      selectedWeather = sunny;
      break;
    case 1:
      selectedWeather = partlyCloudy;
      break;
    case 2:
      selectedWeather = cloudy;
      break;
    case 3:
      selectedWeather = rainy;
      break;
    case 4:
      selectedWeather = stormy;
      break;
    case 5:
      selectedWeather = snowy;
      break;
    case 6:
      selectedWeather = windy;
      break;
    // Night weather (7-11)
    case 7:
      selectedWeather = clearNight;
      break;
    case 8:
      selectedWeather = crescentMoon;
      break;
    case 9:
      selectedWeather = cloudyNight;
      break;
    case 10:
      selectedWeather = rainyNight;
      break;
    case 11:
      selectedWeather = starryNight;
      break;
    default:
      selectedWeather = sunny;
      weatherIndex = 0;
  }

  // Select appropriate color based on day/night
  if (isDay) {
    int dayColorIndex = colorIndex % 4;
    createColorPattern(selectedWeather, dayColors[dayColorIndex][0], dayColors[dayColorIndex][2], dayColors[dayColorIndex][1], pattern);
  } else {
    int nightColorIndex = colorIndex % 3;
    createColorPattern(selectedWeather, nightColors[nightColorIndex][0], nightColors[nightColorIndex][2], nightColors[nightColorIndex][1], pattern);
  }

  // Display the pattern multiple times
  for (int i = 0; i < 10; i++) {
    displayPattern(pattern);
  }

  // Cycle to next color
  colorIndex++;
  int maxColors = isDay ? 4 : 3;
  if (colorIndex >= maxColors) {
    colorIndex = 0;
    weatherIndex++;
    if (weatherIndex >= 12) {
      weatherIndex = 0;
    }
  }
}
