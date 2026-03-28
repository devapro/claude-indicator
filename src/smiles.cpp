#include "smiles.h"
#include "display.h"

// Smiley face definitions (8x8 patterns)

// Happy smile
static uint8_t happySmile[8] = {
  0x3C,  // 00111100
  0x42,  // 01000010
  0xA5,  // 10100101
  0x81,  // 10000001
  0xA5,  // 10100101
  0x99,  // 10011001
  0x42,  // 01000010
  0x3C   // 00111100
};

// Big smile
static uint8_t bigSmile[8] = {
  0x3C,  // 00111100
  0x42,  // 01000010
  0xA5,  // 10100101
  0x81,  // 10000001
  0x81,  // 10000001
  0xC3,  // 11000011
  0x7E,  // 01111110
  0x3C   // 00111100
};

// Wink smile
static uint8_t winkSmile[8] = {
  0x3C,  // 00111100
  0x42,  // 01000010
  0xA5,  // 10100101
  0x81,  // 10000001
  0x99,  // 10011001
  0x99,  // 10011001
  0x42,  // 01000010
  0x3C   // 00111100
};

// Laughing smile
static uint8_t laughSmile[8] = {
  0x3C,  // 00111100
  0x42,  // 01000010
  0xBD,  // 10111101
  0x81,  // 10000001
  0x81,  // 10000001
  0xE7,  // 11100111
  0x7E,  // 01111110
  0x3C   // 00111100
};

// Cool smile (with sunglasses)
static uint8_t coolSmile[8] = {
  0x3C,  // 00111100
  0x42,  // 01000010
  0xFF,  // 11111111
  0x81,  // 10000001
  0x81,  // 10000001
  0x99,  // 10011001
  0x42,  // 01000010
  0x3C   // 00111100
};

// Slight smile
static uint8_t slightSmile[8] = {
  0x3C,  // 00111100
  0x42,  // 01000010
  0xA5,  // 10100101
  0x81,  // 10000001
  0x81,  // 10000001
  0xBD,  // 10111101
  0x42,  // 01000010
  0x3C   // 00111100
};

// Sad smile
static uint8_t sadSmile[8] = {
  0x3C,  // 00111100
  0x42,  // 01000010
  0xA5,  // 10100101
  0x81,  // 10000001
  0x99,  // 10011001
  0xA5,  // 10100101
  0x42,  // 01000010
  0x3C   // 00111100
};

// Surprised smile
static uint8_t surprisedSmile[8] = {
  0x3C,  // 00111100
  0x42,  // 01000010
  0xA5,  // 10100101
  0x81,  // 10000001
  0xBD,  // 10111101
  0xBD,  // 10111101
  0x42,  // 01000010
  0x3C   // 00111100
};

// Function to create a color pattern from a smile and color channels
static void createColorPattern(uint8_t smile[8], bool useRed, bool useGreen, bool useBlue, uint8_t output[3][8])
{
  for (int i = 0; i < 8; i++) {
    output[0][i] = useRed ? smile[i] : 0x00;    // Red channel
    output[1][i] = useBlue ? smile[i] : 0x00;   // Blue channel
    output[2][i] = useGreen ? smile[i] : 0x00;  // Green channel
  }
}

// Function to display smiles periodically with different colors
void displaySmilesCycle()
{
  uint8_t pattern[3][8];
  static int smileIndex = 0;
  static int colorIndex = 0;

  // Define colors (R, G, B)
  bool colors[7][3] = {
    {true, false, false},  // Red
    {false, true, false},  // Green
    {false, false, true},  // Blue
    {true, true, false},   // Yellow (Red + Green)
    {true, false, true},   // Magenta (Red + Blue)
    {false, true, true},   // Cyan (Green + Blue)
    {true, true, true}     // White (All colors)
  };

  // Select smile based on index
  uint8_t* selectedSmile;
  switch (smileIndex) {
    case 0:
      selectedSmile = happySmile;
      break;
    case 1:
      selectedSmile = bigSmile;
      break;
    case 2:
      selectedSmile = winkSmile;
      break;
    case 3:
      selectedSmile = laughSmile;
      break;
    case 4:
      selectedSmile = coolSmile;
      break;
    case 5:
      selectedSmile = slightSmile;
      break;
    case 6:
      selectedSmile = sadSmile;
      break;
    case 7:
      selectedSmile = surprisedSmile;
      break;
    default:
      selectedSmile = happySmile;
      smileIndex = 0;
  }

  // Create pattern with current color
  createColorPattern(selectedSmile, colors[colorIndex][0], colors[colorIndex][2], colors[colorIndex][1], pattern);

  // Display the pattern multiple times
  for (int i = 0; i < 10; i++) {
    displayPattern(pattern);
  }

  // Cycle to next color
  colorIndex++;
  if (colorIndex >= 7) {
    colorIndex = 0;
    smileIndex++;
    if (smileIndex >= 8) {
      smileIndex = 0;
    }
  }
}
