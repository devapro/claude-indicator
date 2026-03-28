#include "shapes.h"
#include "display.h"

// Shape definitions (8x8 patterns)
// Circle pattern
static uint8_t circle[8] = {
  0x3C,  // 00111100
  0x7E,  // 01111110
  0xFF,  // 11111111
  0xFF,  // 11111111
  0xFF,  // 11111111
  0xFF,  // 11111111
  0x7E,  // 01111110
  0x3C   // 00111100
};

// Rectangle pattern
static uint8_t rectangle[8] = {
  0xFF,  // 11111111
  0xFF,  // 11111111
  0xFF,  // 11111111
  0xFF,  // 11111111
  0xFF,  // 11111111
  0xFF,  // 11111111
  0xFF,  // 11111111
  0xFF   // 11111111
};

// Plus (+) pattern
static uint8_t plus[8] = {
  0x18,  // 00011000
  0x18,  // 00011000
  0x18,  // 00011000
  0xFF,  // 11111111
  0xFF,  // 11111111
  0x18,  // 00011000
  0x18,  // 00011000
  0x18   // 00011000
};

// Function to create a color pattern from a shape and color channels
static void createColorPattern(uint8_t shape[8], bool useRed, bool useGreen, bool useBlue, uint8_t output[3][8])
{
  for (int i = 0; i < 8; i++) {
    output[0][i] = useRed ? shape[i] : 0x00;    // Red channel
    output[1][i] = useBlue ? shape[i] : 0x00;   // Blue channel
    output[2][i] = useGreen ? shape[i] : 0x00;  // Green channel
  }
}

// Function to display shapes periodically with different colors
void displayShapesCycle()
{
  uint8_t pattern[3][8];
  static int shapeIndex = 0;
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

  // Select shape based on index
  uint8_t* selectedShape;
  switch (shapeIndex) {
    case 0:
      selectedShape = circle;
      break;
    case 1:
      selectedShape = rectangle;
      break;
    case 2:
      selectedShape = plus;
      break;
    default:
      selectedShape = circle;
      shapeIndex = 0;
  }

  // Create pattern with current color
  createColorPattern(selectedShape, colors[colorIndex][0], colors[colorIndex][2], colors[colorIndex][1], pattern);

  // Display the pattern multiple times
  for (int i = 0; i < 10; i++) {
    displayPattern(pattern);
  }

  // Cycle to next color
  colorIndex++;
  if (colorIndex >= 7) {
    colorIndex = 0;
    shapeIndex++;
    if (shapeIndex >= 3) {
      shapeIndex = 0;
    }
  }
}
