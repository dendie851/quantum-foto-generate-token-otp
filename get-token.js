// get-token.js
const express = require('express');
const redis = require('redis');

const app = express();
const PORT = 3000;

// Koneksi ke Redis menggunakan nama service Docker 'redis-cache'
const redisClient = redis.createClient({ 
  url: 'redis://redis-cache:6379' 
 });
 
redisClient.on('error', (err) => console.error('❌ Redis Client Error:', err));

async function startApiServer() {
  try {
    // 1. Hubungkan ke database Redis
    await redisClient.connect();
    console.log('📦 API Server sukses terhubung ke Redis Cache Container!');

    app.use(express.json());

    // 2. HTTP GET Endpoint untuk dikonsumsi sistem lain / aplikasi pengguna
    app.get('/api/get-token', async (req, res) => {
      try {
        // SPOP: Mengambil 1 token secara acak (Random) sekaligus menghapusnya (Delete) seketika
        const tokenAcak = await redisClient.sPop('token_pool_quantum');

        // Jika stok token di dalam database Redis sedang kosong
        if (!tokenAcak) {
          return res.status(404).json({
            status: 'error',
            message: 'Stok token habis! Silakan sorot kembali sensor hardware TRNG dengan laser.'
          });
        }

        // Jika token berhasil didapatkan
        console.log(`[API] Token ${tokenAcak} berhasil dikonsumsi dan telah dihapus dari Redis.`);
        
        return res.status(200).json({
          status: 'success',
          token: tokenAcak,
          grade: 'Military & Banking Grade (One-Time Use & Zero-Trust Asset)'
        });

      } catch (dbError) {
        console.error('[API Database Error]:', dbError);
        return res.status(500).json({ status: 'error', message: 'Database Server Error' });
      }
    });

    // 3. Jalankan server Express pada port 3000
    app.listen(PORT, () => {
      console.log(`🚀 API Server berjalan di dalam Docker Container pada port ${PORT}`);
      console.log(`👉 Endpoint siap diakses pada: http://localhost:${PORT}/api/get-token`);
    });

  } catch (error) {
    console.error('❌ Gagal menjalankan API Server:', error);
  }
}

startApiServer();