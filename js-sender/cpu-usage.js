#!/usr/bin/env node

require('dotenv').config({ path: __dirname + '/.env' });
const mqtt = require('mqtt');
const os = require('os');

// MQTT Configuration
const MQTT_BROKER = process.env.MQTT_BROKER;
const MQTT_TOPIC = process.env.MQTT_TOPIC;
const MQTT_USER = process.env.MQTT_USER;
const MQTT_PASSWORD = process.env.MQTT_PASSWORD;

const numCores = os.cpus().length;
let previousCpuTimes = os.cpus().map(cpu => ({ ...cpu.times }));

function getPerCoreUsage() {
  const currentCpuTimes = os.cpus().map(cpu => ({ ...cpu.times }));
  const usages = currentCpuTimes.map((curr, i) => {
    const prev = previousCpuTimes[i];
    const idleDiff = curr.idle - prev.idle;
    const totalDiff =
      (curr.user - prev.user) +
      (curr.nice - prev.nice) +
      (curr.sys - prev.sys) +
      (curr.irq - prev.irq) +
      (curr.idle - prev.idle);
    return totalDiff === 0 ? 0 : Math.min(1, Math.max(0, 1 - idleDiff / totalDiff));
  });
  previousCpuTimes = currentCpuTimes;
  return usages;
}

function generatePerCorePattern(usages) {
  // Each core = one column, bar fills bottom to top
  // Up to 8 cores displayed (columns 0-7)
  const redChannel = new Array(8).fill(0);
  const greenChannel = new Array(8).fill(0);
  const blueChannel = new Array(8).fill(0);

  const coresToShow = Math.min(usages.length, 8);

  for (let col = 0; col < coresToShow; col++) {
    const usage = usages[col];
    const filledRows = Math.round(usage * 8);
    const bitPosition = 7 - col; // bit 7 = leftmost column

    for (let row = 0; row < 8; row++) {
      const rowFromBottom = 7 - row;
      if (rowFromBottom < filledRows) {
        // Color per-row based on height: bottom=green, mid=yellow, top=red
        if (rowFromBottom < 3) {
          greenChannel[row] |= (1 << bitPosition);
        } else if (rowFromBottom < 5) {
          redChannel[row] |= (1 << bitPosition);
          greenChannel[row] |= (1 << bitPosition);
        } else {
          redChannel[row] |= (1 << bitPosition);
        }
      }
    }
  }

  const redHex = redChannel.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');
  const greenHex = greenChannel.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');
  const blueHex = blueChannel.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');

  return redHex + greenHex + blueHex;
}

console.log(`CPU Usage Monitor for 8x8 LED Matrix (${numCores} cores, showing ${Math.min(numCores, 8)})`);
console.log('Each column = one core, bar height = load');
console.log('Colors: green (low) -> yellow (mid) -> red (high)');
console.log('Connecting to MQTT broker...');

const client = mqtt.connect(MQTT_BROKER, {
  username: MQTT_USER,
  password: MQTT_PASSWORD,
  clientId: 'cpu_monitor_' + Math.random().toString(16).slice(2, 8)
});

client.on('connect', () => {
  console.log('Connected to MQTT broker');
  console.log('Sending CPU usage every 1 second. Press Ctrl+C to stop.');
  console.log('');

  // Prime the delta
  getPerCoreUsage();

  setInterval(() => {
    const usages = getPerCoreUsage();
    const pattern = generatePerCorePattern(usages);

    const coreStrs = usages.slice(0, 8).map((u, i) => {
      const pct = (u * 100).toFixed(0).padStart(3);
      const bars = Math.round(u * 8);
      return `C${i}:${pct}% ${'|'.repeat(bars)}${'.'.repeat(8 - bars)}`;
    });
    console.log(coreStrs.join('  '));

    client.publish(MQTT_TOPIC, pattern, { qos: 0 });
  }, 1000);
});

client.on('error', (err) => {
  console.error('MQTT Error:', err.message);
  process.exit(1);
});

process.on('SIGINT', () => {
  console.log('\nStopping CPU monitor...');
  client.end();
  process.exit(0);
});
