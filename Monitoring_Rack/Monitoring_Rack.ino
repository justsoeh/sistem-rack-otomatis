#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <FastLED.h>

// ======================= KONFIGURASI 4 STRIP (pin terbaru) =======================
#define PIN_STRIP1 4    // rak A,B,C,D   -> 64 LED
#define PIN_STRIP2 17   // rak E,F,G     -> 48 LED
#define PIN_STRIP3 18   // rak H,I,J     -> 48 LED
#define PIN_STRIP4 21   // rak K,L,M,N   -> 64 LED

#define NUM_LEDS_STRIP1 64
#define NUM_LEDS_STRIP2 48
#define NUM_LEDS_STRIP3 48
#define NUM_LEDS_STRIP4 64

#define TOTAL_LEDS (NUM_LEDS_STRIP1 + NUM_LEDS_STRIP2 + NUM_LEDS_STRIP3 + NUM_LEDS_STRIP4) // 224

CRGB ledsStrip1[NUM_LEDS_STRIP1];
CRGB ledsStrip2[NUM_LEDS_STRIP2];
CRGB ledsStrip3[NUM_LEDS_STRIP3];
CRGB ledsStrip4[NUM_LEDS_STRIP4];

const char* ssid = "Gudang_IoT";
const char* password = "kwhmeter";
const char* apiUrl = "https://script.google.com/macros/s/AKfycbxLnqplkYAau7NJQGFVMa7fBw45KkYH37yfgPcP8759fRjZHtGHPkdqMGH5SNdW1l5f/exec";

WiFiUDP udp;
const int wizPort = 38899;

String lastOrderId = "";

unsigned long waktuLampuNyala = 0;
const unsigned long batasWaktuNyala = 15 * 60 * 1000;
bool statusLampuAktif = false;

unsigned long waktuTerakhirCekAPI = 0;
const unsigned long intervalCekAPI = 61000;

unsigned long waktuRefreshLed = 0;
const unsigned long intervalRefreshLed = 1000;

// ===== NEW: reconnect WiFi throttling =====
unsigned long waktuTerakhirReconnect = 0;
const unsigned long intervalReconnect = 5000;

bool ledAktif[TOTAL_LEDS] = {false}; // status global 0-223

// ======================= MAPPING WIZ =======================
int getWizNumber(String rak) {
  if (rak.startsWith("A.A")) return 1;
  if (rak.startsWith("A.B")) return 2;
  if (rak.startsWith("A.C")) return 3;
  if (rak.startsWith("A.D")) return 4;
  if (rak.startsWith("A.E")) return 5;
  if (rak.startsWith("A.F")) return 6;
  if (rak.startsWith("A.G")) return 7;
  if (rak.startsWith("A.H")) return 8;
  if (rak.startsWith("A.I")) return 9;
  if (rak.startsWith("A.J")) return 10;
  if (rak.startsWith("A.K")) return 11;
  if (rak.startsWith("A.L")) return 12;
  if (rak.startsWith("A.M")) return 13;
  if (rak.startsWith("A.N")) return 14;
  return 0;
}

String getIpForRak(int rakNumber) {
  if (rakNumber >= 1 && rakNumber <= 14) {
    return "192.168.50." + String(110 + rakNumber);
  }
  return "";
}

// ======================= MAPPING WS2812 (LOKAL PER RAK, 0-15) =======================
// rack A
int getLedIndex_A(String rak) {
  if(rak == "A.A44") return 0; if(rak == "A.A43") return 1; if(rak == "A.A42") return 2; if(rak == "A.A41") return 3;
  if(rak == "A.A34") return 4; if(rak == "A.A33") return 5; if(rak == "A.A32") return 6; if(rak == "A.A31") return 7;
  if(rak == "A.A24") return 8; if(rak == "A.A23") return 9; if(rak == "A.A22") return 10; if(rak == "A.A21") return 11;
  if(rak == "A.A14") return 12; if(rak == "A.A13") return 13; if(rak == "A.A12") return 14; if(rak == "A.A11") return 15;
  return -1;
}
// rack B
int getLedIndex_B(String rak) {
  if(rak == "A.B44") return 0; if(rak == "A.B43") return 1; if(rak == "A.B42") return 2; if(rak == "A.B41") return 3;
  if(rak == "A.B34") return 4; if(rak == "A.B33") return 5; if(rak == "A.B32") return 6; if(rak == "A.B31") return 7;
  if(rak == "A.B24") return 8; if(rak == "A.B23") return 9; if(rak == "A.B22") return 10; if(rak == "A.B21") return 11;
  if(rak == "A.B14") return 12; if(rak == "A.B13") return 13; if(rak == "A.B12") return 14; if(rak == "A.B11") return 15;
  return -1;
}
// rack C
int getLedIndex_C(String rak) {
  if(rak == "A.C44") return 0; if(rak == "A.C43") return 1; if(rak == "A.C42") return 2; if(rak == "A.C41") return 3;
  if(rak == "A.C34") return 4; if(rak == "A.C33") return 5; if(rak == "A.C32") return 6; if(rak == "A.C31") return 7;
  if(rak == "A.C24") return 8; if(rak == "A.C23") return 9; if(rak == "A.C22") return 10; if(rak == "A.C21") return 11;
  if(rak == "A.C14") return 12; if(rak == "A.C13") return 13; if(rak == "A.C12") return 14; if(rak == "A.C11") return 15;
  return -1;
}
// rack D
int getLedIndex_D(String rak) {
  if(rak == "A.D44") return 0; if(rak == "A.D43") return 1; if(rak == "A.D42") return 2; if(rak == "A.D41") return 3;
  if(rak == "A.D34") return 4; if(rak == "A.D33") return 5; if(rak == "A.D32") return 6; if(rak == "A.D31") return 7;
  if(rak == "A.D24") return 8; if(rak == "A.D23") return 9; if(rak == "A.D22") return 10; if(rak == "A.D21") return 11;
  if(rak == "A.D14") return 12; if(rak == "A.D13") return 13; if(rak == "A.D12") return 14; if(rak == "A.D11") return 15;
  return -1;
}
// rack E
int getLedIndex_E(String rak) {
  if(rak == "A.E44") return 0; if(rak == "A.E43") return 1; if(rak == "A.E42") return 2; if(rak == "A.E41") return 3;
  if(rak == "A.E34") return 4; if(rak == "A.E33") return 5; if(rak == "A.E32") return 6; if(rak == "A.E31") return 7;
  if(rak == "A.E24") return 8; if(rak == "A.E23") return 9; if(rak == "A.E22") return 10; if(rak == "A.E21") return 11;
  if(rak == "A.E14") return 12; if(rak == "A.E13") return 13; if(rak == "A.E12") return 14; if(rak == "A.E11") return 15;
  return -1;
}
// rack F
int getLedIndex_F(String rak) {
  if(rak == "A.F44") return 0; if(rak == "A.F43") return 1; if(rak == "A.F42") return 2; if(rak == "A.F41") return 3;
  if(rak == "A.F34") return 4; if(rak == "A.F33") return 5; if(rak == "A.F32") return 6; if(rak == "A.F31") return 7;
  if(rak == "A.F24") return 8; if(rak == "A.F23") return 9; if(rak == "A.F22") return 10; if(rak == "A.F21") return 11;
  if(rak == "A.F14") return 12; if(rak == "A.F13") return 13; if(rak == "A.F12") return 14; if(rak == "A.F11") return 15;
  return -1;
}
// rack G
int getLedIndex_G(String rak) {
  if(rak == "A.G44") return 0; if(rak == "A.G43") return 1; if(rak == "A.G42") return 2; if(rak == "A.G41") return 3;
  if(rak == "A.G34") return 4; if(rak == "A.G33") return 5; if(rak == "A.G32") return 6; if(rak == "A.G31") return 7;
  if(rak == "A.G24") return 8; if(rak == "A.G23") return 9; if(rak == "A.G22") return 10; if(rak == "A.G21") return 11;
  if(rak == "A.G14") return 12; if(rak == "A.G13") return 13; if(rak == "A.G12") return 14; if(rak == "A.G11") return 15;
  return -1;
}
// rack H
int getLedIndex_H(String rak) {
  if(rak == "A.H44") return 0; if(rak == "A.H43") return 1; if(rak == "A.H42") return 2; if(rak == "A.H41") return 3;
  if(rak == "A.H34") return 4; if(rak == "A.H33") return 5; if(rak == "A.H32") return 6; if(rak == "A.H31") return 7;
  if(rak == "A.H24") return 8; if(rak == "A.H23") return 9; if(rak == "A.H22") return 10; if(rak == "A.H21") return 11;
  if(rak == "A.H14") return 12; if(rak == "A.H13") return 13; if(rak == "A.H12") return 14; if(rak == "A.H11") return 15;
  return -1;
}
// rack I
int getLedIndex_I(String rak) {
  if(rak == "A.I44") return 0; if(rak == "A.I43") return 1; if(rak == "A.I42") return 2; if(rak == "A.I41") return 3;
  if(rak == "A.I34") return 4; if(rak == "A.I33") return 5; if(rak == "A.I32") return 6; if(rak == "A.I31") return 7;
  if(rak == "A.I24") return 8; if(rak == "A.I23") return 9; if(rak == "A.I22") return 10; if(rak == "A.I21") return 11;
  if(rak == "A.I14") return 12; if(rak == "A.I13") return 13; if(rak == "A.I12") return 14; if(rak == "A.I11") return 15;
  return -1;
}
// rack J
int getLedIndex_J(String rak) {
  if(rak == "A.J44") return 0; if(rak == "A.J43") return 1; if(rak == "A.J42") return 2; if(rak == "A.J41") return 3;
  if(rak == "A.J34") return 4; if(rak == "A.J33") return 5; if(rak == "A.J32") return 6; if(rak == "A.J31") return 7;
  if(rak == "A.J24") return 8; if(rak == "A.J23") return 9; if(rak == "A.J22") return 10; if(rak == "A.J21") return 11;
  if(rak == "A.J14") return 12; if(rak == "A.J13") return 13; if(rak == "A.J12") return 14; if(rak == "A.J11") return 15;
  return -1;
}
// rack K
int getLedIndex_K(String rak) {
  if(rak == "A.K44") return 0; if(rak == "A.K43") return 1; if(rak == "A.K42") return 2; if(rak == "A.K41") return 3;
  if(rak == "A.K34") return 4; if(rak == "A.K33") return 5; if(rak == "A.K32") return 6; if(rak == "A.K31") return 7;
  if(rak == "A.K24") return 8; if(rak == "A.K23") return 9; if(rak == "A.K22") return 10; if(rak == "A.K21") return 11;
  if(rak == "A.K14") return 12; if(rak == "A.K13") return 13; if(rak == "A.K12") return 14; if(rak == "A.K11") return 15;
  return -1;
}
// rack L
int getLedIndex_L(String rak) {
  if(rak == "A.L44") return 0; if(rak == "A.L43") return 1; if(rak == "A.L42") return 2; if(rak == "A.L41") return 3;
  if(rak == "A.L34") return 4; if(rak == "A.L33") return 5; if(rak == "A.L32") return 6; if(rak == "A.L31") return 7;
  if(rak == "A.L24") return 8; if(rak == "A.L23") return 9; if(rak == "A.L22") return 10; if(rak == "A.L21") return 11;
  if(rak == "A.L14") return 12; if(rak == "A.L13") return 13; if(rak == "A.L12") return 14; if(rak == "A.L11") return 15;
  return -1;
}
// rack M — tetap dari atas (tidak dibalik)
int getLedIndex_M(String rak) {
  if(rak == "A.M11") return 0; if(rak == "A.M12") return 1; if(rak == "A.M13") return 2; if(rak == "A.M14") return 3;
  if(rak == "A.M21") return 4; if(rak == "A.M22") return 5; if(rak == "A.M23") return 6; if(rak == "A.M24") return 7;
  if(rak == "A.M31") return 8; if(rak == "A.M32") return 9; if(rak == "A.M33") return 10; if(rak == "A.M34") return 11;
  if(rak == "A.M41") return 12; if(rak == "A.M42") return 13; if(rak == "A.M43") return 14; if(rak == "A.M44") return 15;
  return -1;
}
// rack N
int getLedIndex_N(String rak) {
  if(rak == "A.N44") return 0; if(rak == "A.N43") return 1; if(rak == "A.N42") return 2; if(rak == "A.N41") return 3;
  if(rak == "A.N34") return 4; if(rak == "A.N33") return 5; if(rak == "A.N32") return 6; if(rak == "A.N31") return 7;
  if(rak == "A.N24") return 8; if(rak == "A.N23") return 9; if(rak == "A.N22") return 10; if(rak == "A.N21") return 11;
  if(rak == "A.N14") return 12; if(rak == "A.N13") return 13; if(rak == "A.N12") return 14; if(rak == "A.N11") return 15;
  return -1;
}

// ======================= GABUNGAN: RAK -> INDEX GLOBAL (0-223) =======================
// Offset dasar per rak sesuai urutan strip:
// STRIP1 (pin4):  A=0   B=16  C=32  D=48
// STRIP2 (pin17): E=64  F=80  G=96
// STRIP3 (pin18): H=112 I=128 J=144
// STRIP4 (pin21): K=160 L=176 M=192 N=208
int getGlobalLedIndex(String rak) {
  int local = -1;
  int base = -1;

  if (rak.startsWith("A.A")) { local = getLedIndex_A(rak); base = 0; }
  else if (rak.startsWith("A.B")) { local = getLedIndex_B(rak); base = 16; }
  else if (rak.startsWith("A.C")) { local = getLedIndex_C(rak); base = 32; }
  else if (rak.startsWith("A.D")) { local = getLedIndex_D(rak); base = 48; }
  else if (rak.startsWith("A.E")) { local = getLedIndex_E(rak); base = 64; }
  else if (rak.startsWith("A.F")) { local = getLedIndex_F(rak); base = 80; }
  else if (rak.startsWith("A.G")) { local = getLedIndex_G(rak); base = 96; }
  else if (rak.startsWith("A.H")) { local = getLedIndex_H(rak); base = 112; }
  else if (rak.startsWith("A.I")) { local = getLedIndex_I(rak); base = 128; }
  else if (rak.startsWith("A.J")) { local = getLedIndex_J(rak); base = 144; }
  else if (rak.startsWith("A.K")) { local = getLedIndex_K(rak); base = 160; }
  else if (rak.startsWith("A.L")) { local = getLedIndex_L(rak); base = 176; }
  else if (rak.startsWith("A.M")) { local = getLedIndex_M(rak); base = 192; }
  else if (rak.startsWith("A.N")) { local = getLedIndex_N(rak); base = 208; }

  if (local == -1 || base == -1) return -1;
  return base + local;
}

// ======================= COMMAND WIZ =======================
void sendWizCommand(String ipString, bool turnOn) {
  if (ipString == "") return;

  IPAddress lampIp;
  lampIp.fromString(ipString);

  // NEW: debug print supaya kelihatan command mana yang dikirim ke IP mana
  Serial.println("Kirim WiZ ke " + ipString + " -> " + (turnOn ? "ON" : "OFF"));

  String command = turnOn ?
    "{\"method\":\"setPilot\",\"params\":{\"state\":true,\"r\":0,\"g\":255,\"b\":0,\"dimming\":100}}" :
    "{\"method\":\"setPilot\",\"params\":{\"state\":false}}";

  for (int i = 0; i < 2; i++) {
    udp.beginPacket(lampIp, wizPort);
    udp.print(command);
    udp.endPacket();
    delay(30);
  }
}

// ======================= TULIS 1 LED KE STRIP YANG BENAR =======================
void setPixel(int globalIndex, CRGB color) {
  if (globalIndex < 0 || globalIndex >= TOTAL_LEDS) return;

  if (globalIndex < NUM_LEDS_STRIP1) {
    ledsStrip1[globalIndex] = color;
  } else if (globalIndex < NUM_LEDS_STRIP1 + NUM_LEDS_STRIP2) {
    ledsStrip2[globalIndex - NUM_LEDS_STRIP1] = color;
  } else if (globalIndex < NUM_LEDS_STRIP1 + NUM_LEDS_STRIP2 + NUM_LEDS_STRIP3) {
    ledsStrip3[globalIndex - NUM_LEDS_STRIP1 - NUM_LEDS_STRIP2] = color;
  } else {
    ledsStrip4[globalIndex - NUM_LEDS_STRIP1 - NUM_LEDS_STRIP2 - NUM_LEDS_STRIP3] = color;
  }
}

void showAllStrips() {
  FastLED.show(); // FastLED.show() otomatis update semua strip yang terdaftar
}

void clearAllStrips() {
  fill_solid(ledsStrip1, NUM_LEDS_STRIP1, CRGB::Black);
  fill_solid(ledsStrip2, NUM_LEDS_STRIP2, CRGB::Black);
  fill_solid(ledsStrip3, NUM_LEDS_STRIP3, CRGB::Black);
  fill_solid(ledsStrip4, NUM_LEDS_STRIP4, CRGB::Black);
}

// ======================= MATI LAMPU =======================
void turnOffAllLights() {
  Serial.println("Cleanup: Mematikan semua indikator...");

  for (int i = 1; i <= 14; i++) {
    sendWizCommand(getIpForRak(i), false);
    delay(20);
  }

  for (int i = 0; i < TOTAL_LEDS; i++) {
    ledAktif[i] = false;
  }

  clearAllStrips();
  showAllStrips();
}

// ======================= REFRESH WS2812 =======================
void refreshLedState() {
  clearAllStrips();

  for (int i = 0; i < TOTAL_LEDS; i++) {
    if (ledAktif[i]) {
      setPixel(i, CRGB::Green);
    }
  }

  showAllStrips();
}

// ======================= NEW: PROSES SATU NAMA RAK (setelah displit) =======================
void prosesSatuRak(String namaRak, bool &adaLampuMenyala) {
  namaRak.trim(); // buang spasi tersembunyi di depan/belakang
  if (namaRak.length() == 0) return;

  Serial.println("- Proses rak: [" + namaRak + "]");

  int noWiz = getWizNumber(namaRak);
  Serial.println("  -> noWiz: " + String(noWiz));
  if (noWiz > 0) {
    sendWizCommand(getIpForRak(noWiz), true);
    adaLampuMenyala = true;
  }

  int globalIndex = getGlobalLedIndex(namaRak);
  Serial.println("  -> globalIndex: " + String(globalIndex));
  if (globalIndex != -1) {
    ledAktif[globalIndex] = true;
    adaLampuMenyala = true;
  }
}

// ======================= NEW: SPLIT RAK GABUNGAN "A.A12;A.A13" =======================
void prosesRakMentah(String rakMentah, bool &adaLampuMenyala) {
  int start = 0;
  int len = rakMentah.length();

  while (start < len) {
    int idx = rakMentah.indexOf(';', start);
    String namaRak = (idx == -1) ? rakMentah.substring(start) : rakMentah.substring(start, idx);
    prosesSatuRak(namaRak, adaLampuMenyala);
    if (idx == -1) break;
    start = idx + 1;
  }
}

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, PIN_STRIP1, GRB>(ledsStrip1, NUM_LEDS_STRIP1);
  FastLED.addLeds<WS2812B, PIN_STRIP2, GRB>(ledsStrip2, NUM_LEDS_STRIP2);
  FastLED.addLeds<WS2812B, PIN_STRIP3, GRB>(ledsStrip3, NUM_LEDS_STRIP3);
  FastLED.addLeds<WS2812B, PIN_STRIP4, GRB>(ledsStrip4, NUM_LEDS_STRIP4);

  FastLED.setBrightness(100);
  clearAllStrips();
  showAllStrips();

  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan WiFi");
  int percobaan = 0;
  while (WiFi.status() != WL_CONNECTED && percobaan < 40) {
    delay(500);
    Serial.print(".");
    percobaan++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nGagal connect WiFi saat setup, akan dicoba lagi di loop().");
  }

  udp.begin(wizPort);
}

// ======================= LOOP =======================
void loop() {
  unsigned long waktuSekarang = millis();

  // ===== NEW: reconnect WiFi kalau putus, dicek tiap 5 detik biar tidak spam =====
  if (WiFi.status() != WL_CONNECTED) {
    if (waktuSekarang - waktuTerakhirReconnect >= intervalReconnect) {
      waktuTerakhirReconnect = waktuSekarang;
      Serial.println("WiFi terputus, mencoba reconnect...");
      WiFi.reconnect();
    }
  }

  // ===== refresh WS2812 tiap 1 detik selama status aktif =====
  if (statusLampuAktif && (waktuSekarang - waktuRefreshLed >= intervalRefreshLed)) {
    waktuRefreshLed = waktuSekarang;
    refreshLedState();
  }

  // ===== auto off setelah 15 menit =====
  if (statusLampuAktif && (waktuSekarang - waktuLampuNyala >= batasWaktuNyala)) {
    turnOffAllLights();
    statusLampuAktif = false;
  }

  // ===== cek API tiap 61 detik =====
  if (waktuSekarang - waktuTerakhirCekAPI >= intervalCekAPI || waktuTerakhirCekAPI == 0) {
    waktuTerakhirCekAPI = waktuSekarang;

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;

      if (http.begin(client, apiUrl)) {
        http.setTimeout(10000); // NEW: 10 detik timeout, mencegah ESP32 "hang"
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
          String payload = http.getString();

          // ===== DEBUG: print payload mentah =====
          Serial.println("=== Payload diterima (kode " + String(httpResponseCode) + ") ===");
          Serial.println(payload);
          Serial.println("================================");

          DynamicJsonDocument doc(8192); // NEW: kapasitas dibesarkan dari 4096
          DeserializationError error = deserializeJson(doc, payload);

          if (!error) {
            String currentOrderId = doc["order_id"].as<String>();
            Serial.println("Order ID diterima: " + currentOrderId);

            if (currentOrderId != lastOrderId && currentOrderId != "null") {
              Serial.println("Update Order: " + currentOrderId);

              turnOffAllLights();
              delay(500);

              JsonArray rakArray = doc["rak_komplit"];
              bool adaLampuMenyala = false;

              Serial.println("Daftar Rak yang diorder (" + String(rakArray.size()) + " entri):");

              for (JsonVariant v : rakArray) {
                String rakMentah = v.as<String>();
                // NEW: split otomatis kalau ada gabungan "A.A12;A.A13"
                prosesRakMentah(rakMentah, adaLampuMenyala);
              }

              if (adaLampuMenyala) {
                refreshLedState();
                waktuLampuNyala = millis();
                waktuRefreshLed = millis();
                statusLampuAktif = true;
              } else {
                Serial.println("PERINGATAN: order ada tapi tidak ada rak valid yang cocok mapping.");
              }

              lastOrderId = currentOrderId;
            }
          } else {
            // ===== DEBUG: kalau JSON gagal di-parse =====
            Serial.println("GAGAL parsing JSON! Error: " + String(error.c_str()));
          }
        } else {
          Serial.println("HTTP GET gagal, kode: " + String(httpResponseCode));
        }
        http.end();
      } else {
        Serial.println("http.begin() gagal, cek URL/koneksi.");
      }
    }
  }
}
