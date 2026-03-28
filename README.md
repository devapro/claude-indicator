# ESP8266 LED Matrix Display

8x8 RGB LED Matrix display controlled by ESP8266/NodeMCU with WiFi and MQTT support.

## Features

- 8x8 RGB LED matrix control via SPI
- WiFi connectivity with status indication
- MQTT support for receiving display patterns
- Visual status indicators:
  - Red X: WiFi disconnected
  - Yellow: WiFi connected, MQTT connecting
  - Green checkmark: MQTT ready, waiting for data
- Direct pattern control via MQTT

## Hardware Setup

- **Board**: NodeMCU (ESP8266)
- **LED Matrix**: 8x8 RGB matrix
- **Connection**: SPI (CE pin on D10)

## Configuration

Edit `src/config.h` to configure your WiFi and MQTT settings:

```cpp
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC_DISPLAY "matrix/display"
```

## Building and Uploading

```bash
# Build the project
pio run

# Upload to device
pio run -t upload

# Monitor serial output
pio device monitor
```

## MQTT Pattern Control

### Topics

- **Subscribe**: `matrix/display` - Receives pattern data
- **Publish**: `matrix/status` - Device status updates ("online" on connect)

### Message Format

Send 48 hexadecimal characters representing RGB pattern data:

**Format**: `RRRRRRRRRRRRRRRRGGGGGGGGGGGGGGGGBBBBBBBBBBBBBBBB`

- First 16 chars (8 bytes): Red channel
- Next 16 chars (8 bytes): Green channel
- Last 16 chars (8 bytes): Blue channel

Each byte represents one row of the 8x8 matrix.

#### Why 48 Hex Characters?

The matrix has **64 LEDs (8×8)**, but we use **bitmap** format (each LED is ON or OFF per color channel):

- **Red channel**: 64 LEDs = 64 bits = 8 bytes = 16 hex characters
- **Green channel**: 64 LEDs = 64 bits = 8 bytes = 16 hex characters
- **Blue channel**: 64 LEDs = 64 bits = 8 bytes = 16 hex characters
- **Total**: 16 + 16 + 16 = **48 hex characters**

Each LED can display 8 colors based on which channels are ON:
- `000` = Off (black)
- `100` = Red
- `010` = Green
- `001` = Blue
- `110` = Yellow (Red + Green)
- `101` = Magenta (Red + Blue)
- `011` = Cyan (Green + Blue)
- `111` = White (all channels)

### Examples

```bash
# Red square (all red LEDs on)
mosquitto_pub -h broker.hivemq.com -t "matrix/display" \
  -m "FFFFFFFFFFFFFFFF00000000000000000000000000000000"

# Green heart pattern
mosquitto_pub -h broker.hivemq.com -t "matrix/display" \
  -m "000000000000000000669FFF7E3C180000000000000000"

# Blue checkered pattern
mosquitto_pub -h broker.hivemq.com -t "matrix/display" \
  -m "000000000000000000000000000000AA55AA55AA55AA55"

# Yellow diagonal (red + green)
mosquitto_pub -h broker.hivemq.com -t "matrix/display" \
  -m "81422418811224428142241881122442"

# White X pattern (all colors)
mosquitto_pub -h broker.hivemq.com -t "matrix/display" \
  -m "8142241818244281814224181824428181422418182442818142241818244281"
```

## Project Structure

```
src/
├── main.cpp              # Main application with status icons
├── config.h              # WiFi and MQTT configuration
├── display.h/cpp         # Low-level SPI display control
├── wifi_manager.h/cpp    # WiFi connection management
├── mqtt_manager.h/cpp    # MQTT client handling
├── pattern_parser.h/cpp  # MQTT message parsing
├── status_icons.h/cpp    # Connection status icons
├── emoji.h/cpp           # Example: Emoji patterns (reference)
├── smiles.h/cpp          # Example: Smiley patterns (reference)
├── weather.h/cpp         # Example: Weather patterns (reference)
└── shapes.h/cpp          # Example: Shape patterns (reference)
```

## Creating Custom Patterns

Patterns are 8x8 bitmaps where each bit represents an LED:

```
0x3C = 00111100 = . . # # # # . .
0x42 = 01000010 = . # . . . . # .
0x81 = 10000001 = # . . . . . . #
```

Use tools like https://gurgleapps.com/tools/matrix to design patterns visually.

## Status Indicators

The display shows different icons based on connection state:

1. **White Circle** - Loading/startup (2 seconds on boot)
2. **Cyan Dots** - WiFi connecting attempt (shown continuously during connection)
3. **Red X** - WiFi connection error (2 seconds after each failed attempt)
4. **Yellow Checkmark** - WiFi connected successfully (2 seconds)
5. **Yellow WiFi Signal** - MQTT connecting
6. **Blue X** - MQTT connection error (5 seconds after 3 failed attempts)
7. **Green Checkmark** - MQTT connected, ready to receive data
8. **Pattern Display** - Shows received pattern data

## JS Sender Scripts

The [js-sender/](js-sender/) directory contains Node.js scripts for sending patterns to the matrix over MQTT.

### Setup

```bash
cd js-sender
npm install
```

### sender.js — Pattern Sender

Send predefined or custom patterns to the matrix.

```bash
# Show available patterns
node sender.js

# Send a predefined pattern
node sender.js red
node sender.js heart
node sender.js smile

# Send a custom 48-char hex pattern
node sender.js custom FFFFFFFFFFFFFFFF00000000000000000000000000000000
```

Available patterns: `red`, `green`, `blue`, `yellow`, `heart`, `smile`, `checkmark`, `x`, `frame`, `cross`, `custom`.

### crypto.js — Crypto Market Heatmap

Fetches top 64 cryptocurrencies from CoinGecko and displays a market heatmap on the matrix. Each LED represents one coin (ranked by market cap, left-to-right, top-to-bottom).

```bash
node crypto.js
```

- Green = price up (24h)
- Red = price down (24h)
- Blue = no change

### crypto-detailed.js — Crypto Market Heatmap (Detailed)

Same as `crypto.js` but prints a detailed table with prices, 24h changes, market caps, and top 5 gainers/losers before sending the pattern.

```bash
node crypto-detailed.js
```

### cpu-usage.js — CPU Usage Monitor

Displays per-core CPU usage as a real-time bar chart on the matrix. Each column represents one core (up to 8), bar height indicates load. Updates every second.

```bash
node cpu-usage.js
# Press Ctrl+C to stop
```

- Green (bottom) = low load
- Yellow (middle) = medium load
- Red (top) = high load

### claude-status.js — Claude Code Status Indicator

Monitors Claude Code's state via a file (`/tmp/claude-matrix-state`) and shows animated status on the matrix. Designed to be used with Claude Code hooks.

```bash
node claude-status.js
# Press Ctrl+C to stop
```

States (write to `/tmp/claude-matrix-state`):
- `working` — spinning animation (magenta/blue)
- `permission` — blinking exclamation mark (yellow)
- `waiting` — blinking prompt cursor (green/yellow)
- `idle` — blank display

## Serial Debug Output

Connect at 115200 baud to see:
- WiFi connection status
- MQTT connection status
- Received pattern data
- Pattern parsing status

## License

MIT License
