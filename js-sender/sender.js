#!/usr/bin/env node

require('dotenv').config({ path: __dirname + '/.env' });
const mqtt = require('mqtt');

// MQTT Configuration
const MQTT_BROKER = process.env.MQTT_BROKER;
const MQTT_TOPIC = process.env.MQTT_TOPIC;
const MQTT_USER = process.env.MQTT_USER;
const MQTT_PASSWORD = process.env.MQTT_PASSWORD;

// Connect to MQTT broker
const client = mqtt.connect(MQTT_BROKER, {
  username: MQTT_USER,
  password: MQTT_PASSWORD,
  clientId: 'matrix_sender_' + Math.random().toString(16).slice(2, 8)
});

client.on('connect', () => {
  console.log('Connected to MQTT broker');
  console.log('Topic:', MQTT_TOPIC);
  console.log('');

  // Check command line arguments
  const args = process.argv.slice(2);

  if (args.length === 0) {
    showUsage();
    process.exit(0);
  }

  const command = args[0];

  switch (command) {
    case 'red':
      sendPattern(patterns.redSquare, 'Red Square');
      break;
    case 'green':
      sendPattern(patterns.greenSquare, 'Green Square');
      break;
    case 'blue':
      sendPattern(patterns.blueSquare, 'Blue Square');
      break;
    case 'yellow':
      sendPattern(patterns.yellowSquare, 'Yellow Square');
      break;
    case 'heart':
      sendPattern(patterns.heart, 'Red Heart');
      break;
    case 'smile':
      sendPattern(patterns.smile, 'Yellow Smile');
      break;
    case 'checkmark':
      sendPattern(patterns.checkmark, 'Green Checkmark');
      break;
    case 'x':
      sendPattern(patterns.xMark, 'Red X');
      break;
    case 'frame':
      sendPattern(patterns.frame, 'White Frame');
      break;
    case 'cross':
      sendPattern(patterns.cross, 'Blue Cross');
      break;
    case 'custom':
      if (args.length < 2) {
        console.error('Error: custom pattern requires hex string (48 characters)');
        console.log('Usage: node sender.js custom RRRRRRRRRRRRRRRRGGGGGGGGGGGGGGGGBBBBBBBBBBBBBBBB');
        process.exit(1);
      }
      sendPattern(args[1], 'Custom Pattern');
      break;
    default:
      console.error('Unknown command:', command);
      showUsage();
      process.exit(1);
  }
});

client.on('error', (err) => {
  console.error('MQTT Error:', err.message);
  process.exit(1);
});

function sendPattern(pattern, name) {
  if (pattern.length !== 48) {
    console.error('Error: Pattern must be exactly 48 hex characters');
    console.error('Got:', pattern.length, 'characters');
    process.exit(1);
  }

  console.log('Sending:', name);
  console.log('Pattern:', pattern);

  client.publish(MQTT_TOPIC, pattern, { qos: 1 }, (err) => {
    if (err) {
      console.error('Publish error:', err);
      process.exit(1);
    }
    console.log('✓ Pattern sent successfully!');
    client.end();
  });
}

function showUsage() {
  console.log('ESP8266 Matrix Display Pattern Sender');
  console.log('');
  console.log('Usage: node sender.js <pattern>');
  console.log('');
  console.log('Available patterns:');
  console.log('  red        - Red filled square');
  console.log('  green      - Green filled square');
  console.log('  blue       - Blue filled square');
  console.log('  yellow     - Yellow filled square');
  console.log('  heart      - Red heart shape');
  console.log('  smile      - Yellow smiley face');
  console.log('  checkmark  - Green checkmark');
  console.log('  x          - Red X mark');
  console.log('  frame      - White frame border');
  console.log('  cross      - Blue cross/plus sign');
  console.log('  custom     - Send custom hex pattern (48 chars)');
  console.log('');
  console.log('Examples:');
  console.log('  node sender.js red');
  console.log('  node sender.js smile');
  console.log('  node sender.js custom FFFFFFFFFFFFFFFF00000000000000000000000000000000');
  console.log('');
  console.log('Pattern format: RRRRRRRRRRRRRRRRGGGGGGGGGGGGGGGGBBBBBBBBBBBBBBBB');
  console.log('  - 16 hex chars for Red channel (8 bytes)');
  console.log('  - 16 hex chars for Green channel (8 bytes)');
  console.log('  - 16 hex chars for Blue channel (8 bytes)');
}

// Predefined patterns (48 hex characters each)
const patterns = {
  // Solid colors
  redSquare: 'FFFFFFFFFFFFFFFF' + '0000000000000000' + '0000000000000000',
  greenSquare: '0000000000000000' + 'FFFFFFFFFFFFFFFF' + '0000000000000000',
  blueSquare: '0000000000000000' + '0000000000000000' + 'FFFFFFFFFFFFFFFF',
  yellowSquare: 'FFFFFFFFFFFFFFFF' + 'FFFFFFFFFFFFFFFF' + '0000000000000000',

  // Heart (red)
  heart: '0066FFFF7E3C1800' + '0000000000000000' + '0000000000000000',

  // Smiley face (yellow)
  smile: '3C4299A5A599423C' + '3C4299A5A599423C' + '0000000000000000',

  // Checkmark (green) - centered V shape
  checkmark: '0000000000000000' + '0000814224180000' + '0000000000000000',

  // X mark (red)
  xMark: '8142241818244281' + '0000000000000000' + '0000000000000000',

  // Frame border (white)
  frame: 'FF818181818181FF' + 'FF818181818181FF' + 'FF818181818181FF',

  // Cross/Plus sign (blue)
  cross: '0000000000000000' + '0000000000000000' + '18181818FF181818'
};
