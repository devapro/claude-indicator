#!/usr/bin/env node

require('dotenv').config({ path: __dirname + '/.env' });
const mqtt = require('mqtt');
const https = require('https');

// MQTT Configuration
const MQTT_BROKER = process.env.MQTT_BROKER;
const MQTT_TOPIC = process.env.MQTT_TOPIC;
const MQTT_USER = process.env.MQTT_USER;
const MQTT_PASSWORD = process.env.MQTT_PASSWORD;

// CoinGecko API endpoint
const COINGECKO_API = 'https://api.coingecko.com/api/v3/coins/markets?vs_currency=usd&order=market_cap_desc&per_page=64&page=1&sparkline=false&price_change_percentage=24h';

console.log('Fetching top 64 cryptocurrencies...');
console.log('API:', COINGECKO_API);
console.log('');

// Fetch crypto data
const options = {
  headers: {
    'User-Agent': 'Mozilla/5.0 (compatible; MatrixDisplay/1.0)'
  }
};

https.get(COINGECKO_API, options, (res) => {
  let data = '';

  console.log('HTTP Status:', res.statusCode);

  if (res.statusCode !== 200) {
    console.error(`Error: API returned status code ${res.statusCode}`);
    process.exit(1);
  }

  res.on('data', (chunk) => {
    data += chunk;
  });

  res.on('end', () => {
    try {
      if (!data) {
        console.error('Error: Empty response from API');
        process.exit(1);
      }

      const cryptos = JSON.parse(data);

      if (!Array.isArray(cryptos)) {
        console.error('Error: API response is not an array');
        console.error('Response:', data.substring(0, 200));
        process.exit(1);
      }

      if (cryptos.length === 0) {
        console.error('Error: No crypto data received');
        process.exit(1);
      }

      console.log(`Received ${cryptos.length} cryptocurrencies`);
      console.log('');

      // Display summary
      const positive = cryptos.filter(c => c.price_change_percentage_24h > 0).length;
      const negative = cryptos.filter(c => c.price_change_percentage_24h < 0).length;
      const unchanged = cryptos.filter(c => c.price_change_percentage_24h === 0).length;

      console.log('24h Market Summary:');
      console.log(`  Green (Up):   ${positive} coins`);
      console.log(`  Red (Down):   ${negative} coins`);
      console.log(`  Gray (Same):  ${unchanged} coins`);
      console.log('');

      // Show top movers
      const topGainer = cryptos.reduce((max, c) =>
        c.price_change_percentage_24h > max.price_change_percentage_24h ? c : max
      );
      const topLoser = cryptos.reduce((min, c) =>
        c.price_change_percentage_24h < min.price_change_percentage_24h ? c : min
      );

      console.log('Top Mover (Up):');
      console.log(`  ${topGainer.symbol.toUpperCase()}: +${topGainer.price_change_percentage_24h.toFixed(2)}%`);
      console.log('');
      console.log('Top Mover (Down):');
      console.log(`  ${topLoser.symbol.toUpperCase()}: ${topLoser.price_change_percentage_24h.toFixed(2)}%`);
      console.log('');

      // Generate pattern
      const pattern = generateCryptoPattern(cryptos);

      // Send via MQTT
      sendPattern(pattern);

    } catch (err) {
      console.error('Error parsing crypto data:', err.message);
      process.exit(1);
    }
  });

}).on('error', (err) => {
  console.error('Error fetching crypto data:', err.message);
  process.exit(1);
});

function generateCryptoPattern(cryptos) {
  // Ensure we have exactly 64 entries (8x8 matrix)
  const data = cryptos.slice(0, 64);
  while (data.length < 64) {
    data.push({ price_change_percentage_24h: 0 });
  }

  // Initialize 8x8 arrays for each color channel
  const redChannel = new Array(8).fill(0);
  const greenChannel = new Array(8).fill(0);
  const blueChannel = new Array(8).fill(0);

  // Map each crypto to a pixel
  data.forEach((crypto, index) => {
    const row = Math.floor(index / 8);
    const col = index % 8;
    const change = crypto.price_change_percentage_24h || 0;

    // Set LED bit (bit 7 = leftmost, bit 0 = rightmost)
    const bitPosition = 7 - col;

    if (change > 0) {
      // Positive change = Green
      greenChannel[row] |= (1 << bitPosition);
    } else if (change < 0) {
      // Negative change = Red
      redChannel[row] |= (1 << bitPosition);
    } else {
      // No change = Blue (gray)
      blueChannel[row] |= (1 << bitPosition);
    }
  });

  // Convert to hex pattern (48 characters)
  const redHex = redChannel.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');
  const greenHex = greenChannel.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');
  const blueHex = blueChannel.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');

  return redHex + greenHex + blueHex;
}

function sendPattern(pattern) {
  console.log('Connecting to MQTT broker...');

  const client = mqtt.connect(MQTT_BROKER, {
    username: MQTT_USER,
    password: MQTT_PASSWORD,
    clientId: 'crypto_display_' + Math.random().toString(16).slice(2, 8)
  });

  client.on('connect', () => {
    console.log('Connected to MQTT broker');
    console.log('Sending pattern to matrix...');
    console.log('');
    console.log('Pattern:', pattern);
    console.log('');

    client.publish(MQTT_TOPIC, pattern, { qos: 1 }, (err) => {
      if (err) {
        console.error('Publish error:', err);
        process.exit(1);
      }
      console.log('✓ Crypto market display sent successfully!');
      console.log('');
      console.log('Matrix Layout (left to right, top to bottom):');
      console.log('  Row 1: #1-8   (e.g., BTC, ETH, USDT, BNB, SOL, XRP, USDC, ADA)');
      console.log('  Row 2: #9-16');
      console.log('  Row 3: #17-24');
      console.log('  Row 4: #25-32');
      console.log('  Row 5: #33-40');
      console.log('  Row 6: #41-48');
      console.log('  Row 7: #49-56');
      console.log('  Row 8: #57-64');
      console.log('');
      console.log('Color legend:');
      console.log('  🟢 Green = Price up in last 24h');
      console.log('  🔴 Red   = Price down in last 24h');
      console.log('  🔵 Blue  = No change');
      client.end();
    });
  });

  client.on('error', (err) => {
    console.error('MQTT Error:', err.message);
    process.exit(1);
  });
}
