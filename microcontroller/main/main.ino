#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Konfigurasi Dimensi OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Alamat I2C OLED (Akan dideteksi otomatis oleh program)
uint8_t oled_address = 0x3C; 
bool oled_connected = false;

// Konfigurasi Wi-Fi
const char* ssid = "PutriTunggal";
const char* password = "uu311009";

// Konfigurasi MQTT Broker
const char* mqtt_server = "192.168.100.35";
const int mqtt_port = 1883;
const char* mqtt_topic = "sensor/laser/token";

// Inisialisasi object Wi-Fi dan MQTT
WiFiClient espClient;
PubSubClient client(espClient);

const int ANALOG_SENSOR_PIN = 34; // Pin ADC LDR sesuai gambar

// Variabel untuk menampung token
String token6Digit = "";

// Fungsi untuk mendeteksi alamat I2C OLED secara otomatis
void scanI2C() {
  Wire.begin();
  Serial.println("\n=========================================");
  Serial.println("Memulai Pemindaian Perangkat I2C...");
  Serial.println("=========================================");
  
  bool found = false;
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("-> SUCCESS: Perangkat I2C ditemukan di alamat: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      
      if (address == 0x3C || address == 0x3D) {
        oled_address = address; // Gunakan alamat yang ditemukan
        found = true;
      }
    }
  }
  
  if (!found) {
    Serial.println("-> WARNING: Tidak ada perangkat I2C yang terdeteksi!");
  }
}

// Fungsi untuk menghubungkan ke Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println("\n--- KONEKSI WI-FI ---");
  Serial.print("Menghubungkan ke SSID: ");
  Serial.println(ssid);

  if (oled_connected) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting to WiFi...");
    display.println(ssid);
    display.display();
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[STATUS] Wi-Fi Terhubung Sukses!");
  Serial.print("[STATUS] IP Address ESP32: ");
  Serial.println(WiFi.localIP());

  if (oled_connected) {
    display.println("WiFi Connected!");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
    delay(1500);
  }
}

// Fungsi untuk menghubungkan ulang ke MQTT Broker jika terputus
void reconnect() {
  while (!client.connected()) {
    Serial.print("[MQTT] Mencoba menghubungkan ke Broker...");
    
    if (oled_connected) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Connecting to MQTT...");
      display.display();
    }

    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println(" TERHUBUNG!");
      if (oled_connected) {
        display.println("MQTT Connected!");
        display.display();
        delay(1000);
      }
    } else {
      Serial.print(" GAGAL, rc=");
      Serial.print(client.state());
      Serial.println(" Mencoba lagi dalam 5 detik...");
      
      if (oled_connected) {
        display.println("Failed! Retrying...");
        display.display();
      }
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Jeda peluncuran serial monitor
  analogReadResolution(12); // Memastikan resolusi ESP32 0 - 4095
  
  // Scan alamat I2C terlebih dahulu
  scanI2C();
  
  // Inisialisasi OLED dengan alamat hasil scan otomatis
  if(!display.begin(SSD1306_SWITCHCAPVCC, oled_address)) { 
    Serial.println("\n[ERROR] TAMPILAN OLED GAGAL DIINISIALISASI!");
    Serial.println("[ERROR] Solusi: Periksa kabel VCC/GND/SDA/SCL. Pastikan header pin OLED sudah disolder.");
    Serial.println("===================================================================================\n");
    oled_connected = false;
    // Program tetap berjalan ke fungsi WiFi/MQTT meskipun OLED mati
  } else {
    Serial.println("\n[STATUS] TAMPILAN OLED TERKONEKSI DAN AKTIF!");
    Serial.print("[STATUS] Menggunakan alamat I2C: 0x");
    Serial.println(oled_address, HEX);
    Serial.println("===================================================================================\n");
    oled_connected = true;
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("System Initializing...");
    display.display();
    delay(1000);
  }

  // Menjalankan fungsi koneksi Wi-Fi
  setup_wifi();
  
  // Mengatur server MQTT
  client.setServer(mqtt_server, mqtt_port);

  // Tampilan Standby di OLED jika terhubung
  if (oled_connected) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("=== LASER GENERATOR ===");
    display.println("\nStatus: READY");
    display.println("Menunggu Laser...");
    display.display();
  }

  Serial.println("\nSistem Siap. Silakan sorot dan tahan laser mainan Anda...");
}

void loop() {
  // Pastikan ESP32 tetap terhubung ke MQTT Broker
  if (!client.connected()) {
    reconnect();
    if (oled_connected) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("=== LASER GENERATOR ===");
      display.println("\nStatus: READY");
      display.println("Menunggu Laser...");
      display.display();
    }
  }
  client.loop();

  // Membaca nilai asli murni dari pin ADC
  int nilaiAsli = analogRead(ANALOG_SENSOR_PIN);
  
  // Jika laser terdeteksi (nilai asli drop di bawah 3500)
  if (nilaiAsli < 3500) { 
    Serial.println("\n[ Laser Masuk! Memulai pengambilan 3 sampel otomatis ]");
    
    if (oled_connected) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("--- SAMPLING LDR ---");
      display.display();
    }

    // Reset penampung token di awal proses pembacaan baru
    token6Digit = ""; 
    
    // Perulangan untuk langsung membaca sebanyak 3 kali berturut-turut
    for (int hitungSorotan = 1; hitungSorotan <= 3; hitungSorotan++) {
      int sampelNilai = analogRead(ANALOG_SENSOR_PIN);
      int duaDigit = sampelNilai % 100;
      
      String formatDigit = String(duaDigit);
      if (formatDigit.length() < 2) {
        formatDigit = "0" + formatDigit;
      }
      
      token6Digit += formatDigit;
      
      if (oled_connected) {
        display.print("Sample ");
        display.print(hitungSorotan);
        display.print(": (");
        display.print(sampelNilai);
        display.print(") | ");
        display.println(formatDigit);
        display.display();
      }

      Serial.print("Sampel ke-");
      Serial.print(hitungSorotan);
      Serial.print(" | Nilai Murni: ");
      Serial.print(sampelNilai);
      Serial.print(" | 2 Digit Diambil: ");
      Serial.println(formatDigit);
      
      delay(100); 
    }
    
    Serial.println("--------------------------------");
    Serial.print("👉 TOKEN 6 DIGIT: ");
    Serial.println(token6Digit);
    Serial.println("--------------------------------");
    
    if (oled_connected) {
      display.println("--------------------");
      display.print("Token: ");
      display.println(token6Digit);
      display.display();
    }
    
    Serial.print("Mengirim token ke MQTT...");
    if (client.publish(mqtt_topic, token6Digit.c_str())) {
      Serial.println(" BERHASIL!");
      if (oled_connected) display.println("Pub: Success!");
    } else {
      Serial.println(" GAGAL!");
      if (oled_connected) display.println("Pub: Failed!");
    }
    if (oled_connected) display.display();
    
    Serial.println("Mengunci sistem... Jeda 5 detik sebelum siap membaca lagi.");
    Serial.println("==========================================================");
    delay(5000); 
    
    if (oled_connected) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("=== LASER GENERATOR ===");
      display.println("\nStatus: READY");
      display.println("Menunggu Laser...");
      display.display();
    }

    Serial.println("\nSistem kembali aktif. Menunggu sorotan berikutnya...");
  }
  
  delay(50); 
}