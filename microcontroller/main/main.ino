const int ANALOG_SENSOR_PIN = 34; // Sesuaikan dengan pin yang Anda gunakan (34 atau 36)

// Variabel untuk menampung token
String token6Digit = "";

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // Memastikan resolusi 0 - 4095
  Serial.println("Sistem Siap. Silakan sorot dan tahan laser mainan Anda...");
}

void loop() {
  // Membaca nilai asli murni dari pin ADC
  int nilaiAsli = analogRead(ANALOG_SENSOR_PIN);
  
  // Jika laser terdeteksi (nilai asli drop di bawah 3500)
  if (nilaiAsli < 3500) { 
    Serial.println("\n[ Laser Masuk! Memulai pengambilan 3 sampel otomatis ]");
    
    // Reset penampung token di awal proses pembacaan baru
    token6Digit = ""; 
    
    // Perulangan untuk langsung membaca sebanyak 3 kali berturut-turut
    for (int hitungSorotan = 1; hitungSorotan <= 3; hitungSorotan++) {
      
      // Ambil data analog terbaru pada pin
      int sampelNilai = analogRead(ANALOG_SENSOR_PIN);
      
      // Ambil 2 digit terakhir menggunakan rumus Modulo 100
      int duaDigit = sampelNilai % 100;
      
      // Format agar selalu 2 digit (menambahkan angka 0 di depan jika di bawah 10)
      String formatDigit = String(duaDigit);
      if (formatDigit.length() < 2) {
        formatDigit = "0" + formatDigit;
      }
      
      // Gabungkan ke string token utama
      token6Digit += formatDigit;
      
      // Cetak info tiap sampel yang berhasil diambil
      Serial.print("Sampel ke-");
      Serial.print(hitungSorotan);
      Serial.print(" | Nilai Murni: ");
      Serial.print(sampelNilai);
      Serial.print(" | 2 Digit Diambil: ");
      Serial.println(formatDigit);
      
      // Beri jeda 100 milidetik antar sampel pembacaan agar nilainya bervariasi/fluktuatif
      delay(100); 
    }
    
    // --- SETELAH TOKEN 3 KALI PEMBACAAN SELESAI TERBUAT ---
    Serial.println("--------------------------------");
    Serial.print("👉 TOKEN 6 DIGIT: ");
    Serial.println(token6Digit);
    Serial.println("--------------------------------");
    
    // Memberikan jeda selama 5 detik (5000 milidetik) sesuai permintaan Anda
    Serial.println("Mengunci sistem... Jeda 5 detik sebelum siap membaca lagi.");
    Serial.println("==========================================================");
    delay(5000); 
    
    Serial.println("\nSistem kembali aktif. Menunggu sorotan berikutnya...");
  }
  
  // Jeda kecil standby agar ESP32 tidak panas saat tidak ada laser
  delay(50); 
}