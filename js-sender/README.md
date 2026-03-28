# Matrix Display Pattern Sender

Node.js scripts to send patterns to the ESP8266 Matrix Display via MQTT.

## Installation

```bash
cd js-sender
npm install
```

## Crypto Market Display

Display live cryptocurrency market data on your 8x8 matrix!

### Quick Start

```bash
# Simple crypto display
node crypto.js

# Detailed crypto display with table
node crypto-detailed.js

# Or use npm scripts
npm run crypto
npm run crypto-detailed
```

### What It Shows

The matrix displays the top 64 cryptocurrencies by market cap, with each LED representing one coin:

- **🟢 Green LED** = Price increased in last 24 hours
- **🔴 Red LED** = Price decreased in last 24 hours
- **🔵 Blue LED** = No price change

Layout (left to right, top to bottom):
```
Row 1: #1-8   (BTC, ETH, USDT, BNB, SOL, XRP, USDC, ADA)
Row 2: #9-16  (AVAX, DOGE, TRX, DOT, MATIC, etc.)
Row 3: #17-24
Row 4: #25-32
Row 5: #33-40
Row 6: #41-48
Row 7: #49-56
Row 8: #57-64
```

### Crypto Scripts

**crypto.js** - Simple display with summary
- Shows market overview
- Top gainer and loser
- Sends pattern to matrix

**crypto-detailed.js** - Detailed table view
- Full ranking table with prices
- Top 5 gainers and losers
- Market cap information
- Sends pattern to matrix

### Data Source

Uses the free [CoinGecko API](https://www.coingecko.com/en/api) - no API key required!

## Usage

### Send Predefined Patterns

```bash
node sender.js red         # Red filled square
node sender.js green       # Green filled square
node sender.js blue        # Blue filled square
node sender.js yellow      # Yellow filled square
node sender.js heart       # Red heart
node sender.js smile       # Yellow smiley face
node sender.js checkmark   # Green checkmark
node sender.js x           # Red X mark
node sender.js frame       # White frame border
node sender.js cross       # Blue cross/plus
```

### Send Custom Pattern

```bash
node sender.js custom RRRRRRRRRRRRRRRRGGGGGGGGGGGGGGGGBBBBBBBBBBBBBBBB
```

Example:
```bash
node sender.js custom FFFFFFFFFFFFFFFF00000000000000000000000000000000
```

## Pattern Format

Patterns are 48 hexadecimal characters representing an 8x8 RGB LED matrix:

- **Positions 0-15** (16 chars): Red channel (8 bytes, one per row)
- **Positions 16-31** (16 chars): Green channel (8 bytes, one per row)
- **Positions 32-47** (16 chars): Blue channel (8 bytes, one per row)

Each byte represents one row of 8 LEDs:
- Bit 7 (MSB, leftmost in hex) = LED 0 (leftmost on display)
- Bit 0 (LSB, rightmost in hex) = LED 7 (rightmost on display)

### Example: Red Square

```
Red:   FF FF FF FF FF FF FF FF (all LEDs on)
Green: 00 00 00 00 00 00 00 00 (all LEDs off)
Blue:  00 00 00 00 00 00 00 00 (all LEDs off)

Result: FFFFFFFFFFFFFFFF00000000000000000000000000000000
```

### Example: Custom Heart

```
Pattern visualization:
  01100110  (0x66)
  11111111  (0xFF)
  11111111  (0xFF)
  01111110  (0x7E)
  00111100  (0x3C)
  00011000  (0x18)
  00000000  (0x00)
  00000000  (0x00)

Red:   66 FF FF 7E 3C 18 00 00
Green: 00 00 00 00 00 00 00 00
Blue:  00 00 00 00 00 00 00 00

Command: node sender.js custom 66FFFF7E3C1800000000000000000000000000000000000000000000
```

## Configuration

Edit `sender.js` to change MQTT settings:

```javascript
const MQTT_BROKER = 'mqtt://192.168.100.66:1883';
const MQTT_TOPIC = 'matrix/display';
const MQTT_USER = 'heater';
const MQTT_PASSWORD = 'heater';
```

## Creating New Patterns

1. Design your 8x8 pattern on paper or use https://gurgleapps.com/tools/matrix
2. Convert each row to a hex byte
3. Combine 8 bytes per color channel (16 hex characters)
4. Concatenate: Red + Green + Blue (48 total characters)
5. Send using `node sender.js custom YOUR_PATTERN`

## Examples

### Rainbow Gradient (simplified)
```bash
# Red on left, transitioning to blue on right
node sender.js custom FF7F3F1F0F07030100000F1F3F7F00FFFF
```

### Diagonal Line
```bash
# White diagonal from top-left to bottom-right
node sender.js custom 8040201008040201804020100804020180402010080402018
```
