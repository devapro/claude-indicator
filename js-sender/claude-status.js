#!/usr/bin/env node

require('dotenv').config({ path: __dirname + '/.env' });
const mqtt = require('mqtt');
const fs = require('fs');

// MQTT Configuration
const MQTT_BROKER = process.env.MQTT_BROKER;
const MQTT_TOPIC = process.env.MQTT_TOPIC;
const MQTT_USER = process.env.MQTT_USER;
const MQTT_PASSWORD = process.env.MQTT_PASSWORD;

const STATE_FILE = '/tmp/claude-matrix-state';

// Animation frames for "working" state - spinning pattern
// 20 positions around the ring, 5-pixel trail for smooth animation
const TOTAL_RING_POSITIONS = 20;
const TRAIL_LENGTH = 5;
const SPINNER_FRAMES = Array.from({ length: TOTAL_RING_POSITIONS }, (_, frame) =>
  buildSpinner(Array.from({ length: TRAIL_LENGTH }, (_, i) => frame - TRAIL_LENGTH + 1 + i))
);

// Build a spinner frame - 16 positions around the border of a centered 6x6 area
// Full ring including corners, centered on the 8x8 grid (rows 1-6, cols 1-6)
function buildSpinner(litPositions) {
  // 16 border positions (clockwise from top-left corner)
  const positions = [
    // Top edge (left to right)
    [1, 1], [1, 2], [1, 3], [1, 4], [1, 5], [1, 6],
    // Right edge (top to bottom, excluding corners)
    [2, 6], [3, 6], [4, 6], [5, 6],
    // Bottom edge (right to left)
    [6, 6], [6, 5], [6, 4], [6, 3], [6, 2], [6, 1],
    // Left edge (bottom to top, excluding corners)
    [5, 1], [4, 1], [3, 1], [2, 1],
  ];

  const totalPositions = positions.length;
  const red = new Array(8).fill(0);
  const green = new Array(8).fill(0);
  const blue = new Array(8).fill(0);

  litPositions.forEach((pos, i) => {
    const idx = ((pos % totalPositions) + totalPositions) % totalPositions;
    const [row, col] = positions[idx];
    const bit = 7 - col;
    if (i === litPositions.length - 1) {
      // Head: magenta (red + blue)
      red[row] |= (1 << bit);
      blue[row] |= (1 << bit);
    } else if (i === litPositions.length - 2) {
      // Near head: blue
      blue[row] |= (1 << bit);
    } else {
      // Tail: dim blue
      blue[row] |= (1 << bit);
    }
  });

  return toHex(red, green, blue);
}

// "Permission required" pattern - blinking exclamation mark (yellow)
function buildPermissionFrame(on) {
  const red = new Array(8).fill(0);
  const green = new Array(8).fill(0);
  const blue = new Array(8).fill(0);

  if (on) {
    // Exclamation mark "!" — stem rows 0-4, gap row 5, dot rows 6-7
    // 2px wide, centered at cols 3-4
    //   ..##....   rows 0-4: stem
    //   ..##....
    //   ..##....
    //   ..##....
    //   ..##....
    //   ........   row 5: gap
    //   ..##....   rows 6-7: dot
    //   ..##....
    red[0] = 0b00011000;
    red[1] = 0b00011000;
    red[2] = 0b00011000;
    red[3] = 0b00011000;
    red[4] = 0b00011000;
    red[6] = 0b00011000;
    red[7] = 0b00011000;
    // Yellow = red + green
    green[0] = 0b00011000;
    green[1] = 0b00011000;
    green[2] = 0b00011000;
    green[3] = 0b00011000;
    green[4] = 0b00011000;
    green[6] = 0b00011000;
    green[7] = 0b00011000;
  }

  return toHex(red, green, blue);
}

// "Waiting for input" pattern - blinking cursor with prompt arrow
function buildWaitingFrame(on) {
  const red = new Array(8).fill(0);
  const green = new Array(8).fill(0);
  const blue = new Array(8).fill(0);

  // ">" arrow shape (green), 2px thick for visibility on LED matrix
  // Centered vertically (rows 1-7), left side (cols 0-3)
  //   #.......   row 1
  //   ##......   row 2
  //   .##.....   row 3
  //   ..##....   row 4 (tip)
  //   .##.....   row 5
  //   ##......   row 6
  //   #.......   row 7
  green[1] = 0b10000000;
  green[2] = 0b11000000;
  green[3] = 0b01100000;
  green[4] = 0b00110000;
  green[5] = 0b01100000;
  green[6] = 0b11000000;
  green[7] = 0b10000000;

  if (on) {
    // Cursor block (yellow = red+green) at cols 5-6, rows 2-6
    const cursor = 0b00000110;
    red[2] = cursor;
    red[3] = cursor;
    red[4] = cursor;
    red[5] = cursor;
    red[6] = cursor;
    green[2] |= cursor;
    green[3] |= cursor;
    green[4] |= cursor;
    green[5] |= cursor;
    green[6] |= cursor;
  }

  return toHex(red, green, blue);
}

function toHex(red, green, blue) {
  const r = red.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');
  const g = green.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');
  const b = blue.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');
  return r + g + b;
}

// State management
let currentState = 'idle';
let spinnerFrame = 0;
let cursorOn = true;

function readState() {
  try {
    return fs.readFileSync(STATE_FILE, 'utf8').trim();
  } catch {
    return 'idle';
  }
}

console.log('Claude Code Status Monitor for 8x8 LED Matrix');
console.log(`State file: ${STATE_FILE}`);
console.log('States: "working" = spinner, "waiting" = blinking prompt, "permission" = blinking !');
console.log('Connecting to MQTT broker...');

const client = mqtt.connect(MQTT_BROKER, {
  username: MQTT_USER,
  password: MQTT_PASSWORD,
  clientId: 'claude_status_' + Math.random().toString(16).slice(2, 8)
});

client.on('connect', () => {
  console.log('Connected to MQTT broker');
  console.log('Monitoring state. Press Ctrl+C to stop.');
  console.log('');

  // Write initial state
  fs.writeFileSync(STATE_FILE, 'idle');

  setInterval(() => {
    const newState = readState();
    if (newState !== currentState) {
      console.log(`State: ${currentState} -> ${newState}`);
      currentState = newState;
      spinnerFrame = 0;
      cursorOn = true;
    }

    let pattern;

    if (currentState === 'working') {
      pattern = SPINNER_FRAMES[spinnerFrame % SPINNER_FRAMES.length];
      spinnerFrame++;
    } else if (currentState === 'permission') {
      pattern = buildPermissionFrame(cursorOn);
      cursorOn = !cursorOn;
    } else if (currentState === 'waiting') {
      pattern = buildWaitingFrame(cursorOn);
      cursorOn = !cursorOn;
    } else {
      // idle - blank display
      pattern = '0'.repeat(48);
    }

    client.publish(MQTT_TOPIC, pattern, { qos: 0 });
  }, 150);
});

client.on('error', (err) => {
  console.error('MQTT Error:', err.message);
  process.exit(1);
});

process.on('SIGINT', () => {
  console.log('\nStopping...');
  // Clear display
  client.publish(MQTT_TOPIC, '0'.repeat(48), { qos: 0 }, () => {
    client.end();
    try { fs.unlinkSync(STATE_FILE); } catch {}
    process.exit(0);
  });
});
