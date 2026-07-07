// cache-token.js
const mqtt = require('mqtt');
const redis = require('redis');

// Koneksi ke Redis menggunakan nama service Docker 'redis-cache'
const redisClient = redis.createClient({ 
  url: 'redis://redis-cache:6379' 
});

redisClient.on('error', (err) => console.error('❌ Redis Client Error:', err));

async function startWorker() {
  try {
    // 1. Hubungkan ke database Redis
    await redisClient.connect();
    console.log('📦 Worker sukses terhubung ke Redis Cache Container!');

    // 2. Hubungkan ke MQTT Broker menggunakan nama service Docker 'mqtt-broker'
    const mqttClient = mqtt.connect('mqtt://mqtt-broker:1883');

    mqttClient.on('connect', () => {
      console.log('🛰️  Worker sukses terhubung ke MQTT Broker Container!');
      
      // Subscribe ke topik tempat ESP32 mengirim data token
      mqttClient.subscribe('sensor/laser/token', (err) => {
        if (!err) {
          console.log('🔔 Sukses SUBSCRIBE ke topik: sensor/laser/token');
        } else {
          console.error('❌ Gagal subscribe ke topik MQTT:', err);
        }
      });
    });

    // 3. Logika menangkap token dari MQTT dan menyimpannya ke Redis
    mqttClient.on('message', async (topic, message) => {
      if (topic === 'sensor/laser/token') {
        const tokenBaru = message.toString().trim();
        console.log(`[MQTT] Menerima token dari ESP32: ${tokenBaru}`);

        try {
          // SADD: Memasukkan token ke dalam struktur data Set di Redis
          await redisClient.sAdd('token_pool_quantum', tokenBaru);
          console.log(`[Redis] Token ${tokenBaru} berhasil diamankan ke dalam Set Pool.\n`);
        } catch (redisErr) {
          console.error('[Redis] Gagal menyimpan token ke Set:', redisErr);
        }
      }
    });

  } catch (error) {
    console.error('❌ Batas fatal pada Worker:', error);
  }
}

startWorker();