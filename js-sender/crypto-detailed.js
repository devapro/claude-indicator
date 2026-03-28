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
const COINGECKO_API = 'https://api.coingecko.com/api/v3/coins/markets?vs_currency=usd&order=market_cap_desc&per_page=64&page=1&sparkline=false&price_change_percentage=1h';

console.log('Fetching top 64 cryptocurrencies with details...');
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

      // Print detailed table
      console.log('Rank | Symbol    | Price         | 24h Change | Market Cap');
      console.log('-----|-----------|---------------|------------|------------------');

      cryptos.slice(0, 64).forEach((crypto, index) => {
        const rank = (index + 1).toString().padStart(4, ' ');
        const symbol = crypto.symbol.toUpperCase().padEnd(9, ' ');
        const price = `$${crypto.current_price.toLocaleString('en-US', {
          minimumFractionDigits: 2,
          maximumFractionDigits: crypto.current_price < 1 ? 6 : 2
        })}`.padEnd(13, ' ');
        const change = crypto.price_change_percentage_24h.toFixed(2);
        const changeStr = (change >= 0 ? '+' : '') + change + '%';
        const changeFormatted = changeStr.padStart(10, ' ');
        const marketCap = `$${(crypto.market_cap / 1e9).toFixed(2)}B`;

        console.log(`${rank} | ${symbol} | ${price} | ${changeFormatted} | ${marketCap}`);
      });

      console.log('');

      // Display summary
      const positive = cryptos.filter(c => c.price_change_percentage_24h > 0).length;
      const negative = cryptos.filter(c => c.price_change_percentage_24h < 0).length;

      console.log('24h Market Summary:');
      console.log(`  Gainers: ${positive} (${(positive/cryptos.length*100).toFixed(1)}%)`);
      console.log(`  Losers:  ${negative} (${(negative/cryptos.length*100).toFixed(1)}%)`);
      console.log('');

      // Show top 5 gainers and losers
      const sorted = [...cryptos].sort((a, b) =>
        b.price_change_percentage_24h - a.price_change_percentage_24h
      );

      console.log('Top 5 Gainers:');
      sorted.slice(0, 5).forEach((c, i) => {
        console.log(`  ${i + 1}. ${c.symbol.toUpperCase()}: +${c.price_change_percentage_24h.toFixed(2)}%`);
      });
      console.log('');

      console.log('Top 5 Losers:');
      sorted.slice(-5).reverse().forEach((c, i) => {
        console.log(`  ${i + 1}. ${c.symbol.toUpperCase()}: ${c.price_change_percentage_24h.toFixed(2)}%`);
      });
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
    console.log('Sending crypto market visualization to matrix...');
    console.log('');

    client.publish(MQTT_TOPIC, pattern, { qos: 1 }, (err) => {
      if (err) {
        console.error('Publish error:', err);
        process.exit(1);
      }
      console.log('✓ Pattern sent successfully!');
      console.log('');
      console.log('Your matrix now displays the top 64 cryptocurrencies');
      console.log('  🟢 Green = Price increased');
      console.log('  🔴 Red   = Price decreased');
      console.log('  🔵 Blue  = No change');
      client.end();
    });
  });

  client.on('error', (err) => {
    console.error('MQTT Error:', err.message);
    process.exit(1);
  });
}
