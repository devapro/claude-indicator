#include "emoji.h"
#include "display.h"

// Emoji definitions (8x8 patterns)

// Smiley face emoji
static uint8_t smiley[8] = {
  0x3C,  // 00111100
  0x42,  // 01000010
  0xA5,  // 10100101
  0x81,  // 10000001
  0xA5,  // 10100101
  0x99,  // 10011001
  0x42,  // 01000010
  0x3C   // 00111100
};

// Heart emoji
static uint8_t heart[8] = {
  0x00,  // 00000000
  0x66,  // 01100110
  0xFF,  // 11111111
  0xFF,  // 11111111
  0xFF,  // 11111111
  0x7E,  // 01111110
  0x3C,  // 00111100
  0x18   // 00011000
};

// Star emoji
static uint8_t star[8] = {
  0x18,  // 00011000
  0x18,  // 00011000
  0x5A,  // 01011010
  0x3C,  // 00111100
  0x3C,  // 00111100
  0x5A,  // 01011010
  0x99,  // 10011001
  0x81   // 10000001
};

// Sun emoji
static uint8_t sun[8] = {
  0x99,  // 10011001
  0x5A,  // 01011010
  0x3C,  // 00111100
  0xFF,  // 11111111
  0xFF,  // 11111111
  0x3C,  // 00111100
  0x5A,  // 01011010
  0x99   // 10011001
};

// Music note emoji
static uint8_t music[8] = {
  0x0C,  // 00001100
  0x0E,  // 00001110
  0x0C,  // 00001100
  0x0C,  // 00001100
  0x0C,  // 00001100
  0x6C,  // 01101100
  0x6C,  // 01101100
  0x38   // 00111000
};

// Umbrella emoji
static uint8_t umbrella[8] = {
  0x38,  // 00111000
  0x7C,  // 01111100
  0xFE,  // 11111110
  0xFF,  // 11111111
  0x10,  // 00010000
  0x10,  // 00010000
  0x10,  // 00010000
  0x30   // 00110000
};

// Pizza slice emoji
static uint8_t pizza[8] = {
  0x01,  // 00000001
  0x03,  // 00000011
  0x05,  // 00000101
  0x0B,  // 00001011
  0x15,  // 00010101
  0x2B,  // 00101011
  0x57,  // 01010111
  0xFF   // 11111111
};

// Diamond emoji
static uint8_t diamond[8] = {
  0x18,  // 00011000
  0x3C,  // 00111100
  0x7E,  // 01111110
  0xFF,  // 11111111
  0xFF,  // 11111111
  0x7E,  // 01111110
  0x3C,  // 00111100
  0x18   // 00011000
};

// Function to create a color pattern from an emoji and color channels
static void createColorPattern(uint8_t emoji[8], bool useRed, bool useGreen, bool useBlue, uint8_t output[3][8])
{
  for (int i = 0; i < 8; i++) {
    output[0][i] = useRed ? emoji[i] : 0x00;    // Red channel
    output[1][i] = useBlue ? emoji[i] : 0x00;   // Blue channel
    output[2][i] = useGreen ? emoji[i] : 0x00;  // Green channel
  }
}

// Function to display emojis periodically with different colors
void displayEmojiCycle()
{
  uint8_t pattern[3][8];
  static int emojiIndex = 0;
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

  // Select emoji based on index
  uint8_t* selectedEmoji;
  switch (emojiIndex) {
    case 0:
      selectedEmoji = smiley;
      break;
    case 1:
      selectedEmoji = heart;
      break;
    case 2:
      selectedEmoji = star;
      break;
    case 3:
      selectedEmoji = sun;
      break;
    case 4:
      selectedEmoji = music;
      break;
    case 5:
      selectedEmoji = umbrella;
      break;
    case 6:
      selectedEmoji = pizza;
      break;
    case 7:
      selectedEmoji = diamond;
      break;
    default:
      selectedEmoji = smiley;
      emojiIndex = 0;
  }

  // Create pattern with current color
  createColorPattern(selectedEmoji, colors[colorIndex][0], colors[colorIndex][2], colors[colorIndex][1], pattern);

  // Display the pattern multiple times
  for (int i = 0; i < 10; i++) {
    displayPattern(pattern);
  }

  // Cycle to next color
  colorIndex++;
  if (colorIndex >= 7) {
    colorIndex = 0;
    emojiIndex++;
    if (emojiIndex >= 8) {
      emojiIndex = 0;
    }
  }
}
