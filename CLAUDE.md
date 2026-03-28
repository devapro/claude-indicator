# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a PlatformIO project for ESP8266/NodeMCU that controls an 8x8 RGB LED matrix via SPI. The project displays visual status icons during connection (WiFi, MQTT) and shows RGB patterns received via MQTT. Example pattern files (emoji, smiles, weather, shapes) are included for reference but not used in the main application.

## Build and Development Commands

```bash
# Build the project
pio run

# Upload to device
pio run --target upload

# Build and upload
pio run -t upload

# Monitor serial output
pio device monitor

# Clean build files
pio run --target clean
```

## Hardware Configuration

- **Board**: NodeMCU (ESP8266)
- **LED Matrix**: 8x8 RGB matrix controlled via SPI
- **Pin Configuration**:
  - `CE` (Chip Enable): D10
  - SPI data transfer: Uses default SPI pins

## Configuration

Before building, update `src/config.h` with your WiFi and MQTT broker settings:
- WiFi SSID and password
- MQTT broker address and credentials
- MQTT topics for display commands and status

## Code Architecture

The codebase is organized into modular components:

### Network Layer

#### WiFi Manager (`wifi_manager.h/cpp`)
- **Purpose**: Manages WiFi connectivity
- **Key Functions**:
  - `wifiInit()`: Initialize WiFi in station mode
  - `wifiConnect()`: Connect to configured WiFi network (displays cyan connecting dots continuously during ~10 second connection attempt)
  - `wifiIsConnected()`: Check connection status
  - Handles reconnection attempts with timeout (10 seconds per attempt)
- **Visual feedback**: Cyan dots pattern refreshes continuously during the entire connection process to avoid display blinking
- **Error handling**: After each failed attempt, displays red X for 2 seconds before retrying

#### MQTT Manager (`mqtt_manager.h/cpp`)
- **Purpose**: MQTT client for remote control
- **Key Functions**:
  - `mqttInit(callback)`: Initialize MQTT with message callback
  - `mqttConnect()`: Connect to MQTT broker (tracks failures after 3 attempts)
  - `mqttIsConnected()`: Check if MQTT is connected
  - `mqttConnectionFailed()`: Check if connection failed after multiple attempts
  - `mqttResetFailureState()`: Reset failure state to retry connection
  - `mqttLoop()`: Process MQTT messages (call in main loop)
  - `mqttPublish()`: Send status messages
- **Topics**:
  - Subscribe: `matrix/display` (receive commands)
  - Publish: `matrix/status` (device status)
- **Failure handling**: Marks connection as failed after 3 unsuccessful attempts, displays blue X error icon for 5 seconds before reset
- Automatically reconnects on disconnect

#### Pattern Parser (`pattern_parser.h/cpp`)
- **Purpose**: Parse raw RGB pattern data from MQTT
- **Message Format**: 48 hex characters (16 red + 16 green + 16 blue)
  - Each channel is 8 bytes representing 8 rows
  - Example: `FFFFFFFFFFFFFFFF00000000000000000000000000000000` = red square
- **Key Functions**:
  - `parseAndDisplay()`: Parse 48-char hex string into RGB pattern
  - `hasPatternData()`: Check if data has been received
  - `updateDisplay()`: Display the received pattern (call in loop)

#### Status Icons (`status_icons.h/cpp`)
- **Purpose**: Visual feedback for connection states
- **Icons**:
  - `displayLoading()`: White circle during startup/boot (2 seconds)
  - `displayWifiConnectingFrame()`: Single frame of cyan dots (used during WiFi connection attempts)
  - `displayWifiConnecting()`: Cyan dots displayed continuously during connection attempts
  - `displayWifiErrorFrame()`: Single frame of red X (shown after each failed WiFi attempt)
  - `displayWifiError()`: Red X displayed continuously when WiFi connection attempt fails (2 seconds after each attempt)
  - `displayWifiConnectedFrame()`: Single frame of yellow checkmark (shown when WiFi connects)
  - `displayWifiConnected()`: Yellow checkmark displayed when WiFi successfully connects (2 seconds)
  - `displayMqttConnecting()`: Yellow WiFi signal when connecting to MQTT
  - `displayMqttErrorFrame()`: Single frame of blue X (used when MQTT connection fails)
  - `displayMqttError()`: Blue X displayed continuously when MQTT connection fails (5 seconds, after 3 attempts)
  - `displayMqttReady()`: Green checkmark when ready for data
- **Important**: Different patterns and colors distinguish between connection states:
  - WiFi: Cyan dots (connecting) → Red X (failed, 2s after each attempt) → Yellow checkmark (connected, 2s)
  - MQTT: Yellow signal (connecting) → Blue X (failed after 3 attempts, 5s) → Green checkmark (ready)

### Core Display Layer (`display.h/cpp`)
- **Purpose**: Low-level SPI communication and matrix control
- **Key Variables**:
  - `data[4]`: SPI data buffer for matrix communication
  - `CE`: Chip enable pin (D10)
- **Key Function**: `displayPattern(uint8_t pattern[3][8])`
  - Accepts 3x8 array where indices represent:
    - `[0]`: Red channel pixels
    - `[1]`: Blue channel pixels (note: hardware mapping swaps indices 1 and 2)
    - `[2]`: Green channel pixels
  - Each byte represents one row of 8 LEDs (bit=1 means LED on)
  - Uses inverted logic: `~pattern[c][j]` because hardware is active-low

### Example Pattern Libraries (Reference Only)
These modules are included as examples but NOT used in the main application:
- **`emoji.h/cpp`**: 8 emoji patterns with color cycling
- **`smiles.h/cpp`**: 8 smiley face variations
- **`weather.h/cpp`**: 12 weather icons (day/night)
- **`shapes.h/cpp`**: Basic geometric shapes
- **Pattern structure**: Static 8x8 byte arrays with color cycling logic

### Application Entry (`main.cpp`)
- Initializes hardware (SPI, display, WiFi, MQTT)
- Shows loading pattern for 2 seconds after display initialization
- Sets up MQTT message callback for pattern data
- Tracks WiFi connection state to show yellow checkmark once when connected
- Main loop with state-based display:
  1. **WiFi disconnected**: Attempt reconnect (cyan dots displayed continuously during ~10 second attempt)
  2. **WiFi connection attempt failed**: Show red X error icon for 2 seconds, then retry
  3. **WiFi connected (first time)**: Show yellow checkmark for 2 seconds
  4. **MQTT connection failed** (after 3 attempts): Show blue X error icon for 5 seconds, reset failure state, then retry
  5. **WiFi connected, MQTT disconnected**: Show yellow WiFi signal, attempt MQTT connect
  6. **MQTT connected, no data**: Show green checkmark, wait for data
  7. **Data received**: Display the received pattern continuously
- Serial output at 115200 baud for debugging

## Adding New Patterns

To add new patterns or animations:

1. **Create pattern definition** (8x8 byte array):
   ```cpp
   static uint8_t myPattern[8] = {
     0x18,  // 00011000
     0x3C,  // 00111100
     // ... 6 more rows
   };
   ```
   - Each byte is one row (8 pixels)
   - Binary notation in comments helps visualize the pattern
   - Use tools like https://gurgleapps.com/tools/matrix to generate hex codes

2. **Add to switch statement** in the appropriate cycle function
3. **Update cycle counter** to include the new pattern count

## Pattern Data Format

- **8x8 grid**: Each pattern is 8 bytes (one byte per row)
- **Bit mapping**: Bit 7 (leftmost) = LED 0, Bit 0 (rightmost) = LED 7
- **Color channels**: Patterns are replicated across red/green/blue channels
- **Hardware quirk**: The physical wiring swaps blue and green channel indices in the data buffer

## Module Dependencies

```
main.cpp
  ├─> wifi_manager.h (WiFi connectivity)
  ├─> mqtt_manager.h (MQTT client)
  │    └─> PubSubClient library
  ├─> pattern_parser.h (parse 48-char hex data)
  │    └─> display.h (display parsed pattern)
  ├─> status_icons.h (connection state icons)
  │    └─> display.h
  └─> display.h (SPI control)

Example modules (not used in main):
  - emoji.h/cpp, smiles.h/cpp, weather.h/cpp, shapes.h/cpp

js-sender/ (Node.js MQTT clients)
  ├─> sender.js (send predefined patterns)
  ├─> crypto.js (CoinGecko market visualization)
  ├─> crypto-detailed.js (detailed market data + visualization)
  ├─> cpu-usage.js (real-time CPU monitor)
  └─> claude-status.js (Claude Code status indicator)
      All scripts load config from .env via dotenv
```

The main application does NOT use the example animation modules. It only displays:
1. Status icons based on connection state
2. Raw RGB pattern data received via MQTT

## MQTT Message Format

Send 48 hexadecimal characters to the `matrix/display` topic:

**Format**: `RRRRRRRRRRRRRRRRGGGGGGGGGGGGGGGGBBBBBBBBBBBBBBBB`
- Positions 0-15 (16 chars): Red channel (8 bytes)
- Positions 16-31 (16 chars): Green channel (8 bytes)
- Positions 32-47 (16 chars): Blue channel (8 bytes)

### Why 48 Hex Characters?

The 8×8 matrix has **64 LEDs total**, but we use **bitmap** format (each LED is ON/OFF per color channel):

- **Calculation**: 64 LEDs = 64 bits = 8 bytes = 16 hex characters **per channel**
- **Three channels**: Red (16) + Green (16) + Blue (16) = **48 hex characters**
- **Result**: Each LED can display 8 colors based on RGB combinations:
  - `000` = Off, `100` = Red, `010` = Green, `001` = Blue
  - `110` = Yellow, `101` = Magenta, `011` = Cyan, `111` = White

This bitmap approach is memory-efficient for simple on/off control per channel, allowing 8 possible colors per LED.

## JS Sender Tools (`js-sender/`)

Node.js scripts for sending patterns to the matrix via MQTT. Configuration is loaded from `js-sender/.env` (see `.env.example`).

### Setup

```bash
cd js-sender
cp .env.example .env   # Edit with your MQTT credentials
npm install
```

### Available Scripts

| Command | Script | Description |
|---------|--------|-------------|
| `npm start -- <pattern>` | `sender.js` | Send predefined patterns (red, green, blue, yellow, heart, smile, checkmark, x, frame, cross, custom) |
| `npm run crypto` | `crypto.js` | Fetch top 64 cryptos from CoinGecko, display 24h change as green/red/blue grid |
| `npm run crypto-detailed` | `crypto-detailed.js` | Same as crypto but with detailed table output (prices, market caps, top movers) |
| `npm run cpu-usage` | `cpu-usage.js` | Real-time per-core CPU usage bar chart (updates every 1s, green/yellow/red by load) |
| `npm run claude-status` | `claude-status.js` | Claude Code status indicator — reads state from `/tmp/claude-matrix-state` and shows spinner (working), blinking ! (permission), or blinking cursor (waiting) |

### Environment Variables

| Variable | Description | Example |
|----------|-------------|---------|
| `MQTT_BROKER` | MQTT broker URL | `mqtt://192.168.100.66:1883` |
| `MQTT_TOPIC` | MQTT topic for display commands | `matrix/display` |
| `MQTT_USER` | MQTT username | |
| `MQTT_PASSWORD` | MQTT password | |

## Testing MQTT Commands

```bash
# Using sender.js
cd js-sender
node sender.js red
node sender.js heart
node sender.js custom FFFFFFFFFFFFFFFF00000000000000000000000000000000

# Using mosquitto_pub directly
mosquitto_pub -h 192.168.100.66 -t "matrix/display" \
  -m "FFFFFFFFFFFFFFFF00000000000000000000000000000000"
```
