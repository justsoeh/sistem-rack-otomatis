#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <FastLED.h>

// ======================= KONFIGURASI: 1 RAK = 1 PIN, 16 LED =======================
#define PIN_A 13
#define PIN_B 14
#define PIN_C 27
#define PIN_D 26
#define PIN_E 25
#define PIN_F 33
#define PIN_G 32
#define PIN_H 4
#define PIN_I 16
#define PIN_J 17
#define PIN_K 5
#define PIN_L 18
#define PIN_M 19
#define PIN_N 21

#define LED_PER_RAK 16

CRGB ledsA[LED_PER_RAK];
CRGB ledsB[LED_PER_RAK];
CRGB ledsC[LED_PER_RAK];
CRGB ledsD[LED_PER_RAK];
CRGB ledsE[LED_PER_RAK];
CRGB ledsF[LED_PER_RAK];
CRGB ledsG[LED_PER_RAK];
CRGB ledsH[LED_PER_RAK];
CRGB ledsI[LED_PER_RAK];
CRGB ledsJ[LED_PER_RAK];
CRGB ledsK[LED_PER_RAK];
CRGB ledsL[LED_PER_RAK];
CRGB ledsM[LED_PER_RAK];
CRGB ledsN[LED_PER_RAK];

CRGB* ledArrays[14] = {ledsA, ledsB, ledsC, ledsD, ledsE, ledsF, ledsG, ledsH, ledsI, ledsJ, ledsK, ledsL, ledsM, ledsN};
const char rakUrutan[14] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N'};

bool statusSlot[14][16] = {false};

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

unsigned long waktuTerakhirReconnect = 0;
const unsigned long intervalReconnect = 5000;

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

void sendWizCommand(String ipString, bool turnOn) {
  if (ipString == "") return;
  IPAddress lampIp;
  lampIp.fromString(ipString);
  Serial.println("Kirim WiZ ke " + ipString + " -> " + (turnOn ? "ON" : "OFF"));
  String command = turnOn ?
    "{\"method\":\"setPilot\",\"params\":{\"state\":true,\"r\":255,\"g\":0,\"b\":0,\"dimming\":100}}" :
    "{\"method\":\"setPilot\",\"params\":{\"state\":false}}";
  for (int i = 0; i < 2; i++) {
    udp.beginPacket(lampIp, wizPort);
    udp.print(command);
    udp.endPacket();
    delay(30);
  }
}

// ======================= MAPPING RAK -> INDEX (0-13) DAN SLOT (0-15) =======================
int getRakIndex(String rak) {
  if (rak.length() < 3) return -1;
  char hurufRak = rak.charAt(2);
  for (int i = 0; i < 14; i++) {
    if (rakUrutan[i] == hurufRak) return i;
  }
  return -1;
}

// slot lokal: kebanyakan rak reversed (44->0 ... 11->15), kecuali rak M normal
int getSlotIndex(String rak, int rakIdx) {
  if (rak.length() < 5) return -1;
  String slot = rak.substring(3, 5); // contoh "44"

  char rakHuruf = rakUrutan[rakIdx];
  const char* urutanReversed[16] = {"44","43","42","41","34","33","32","31","24","23","22","21","14","13","12","11"};
  const char* urutanNormal[16]   = {"11","12","13","14","21","22","23","24","31","32","33","34","41","42","43","44"};

  const char** urutan = (rakHuruf == 'M') ? urutanNormal : urutanReversed;

  for (int i = 0; i < 16; i++) {
    if (slot.equals(urutan[i])) return i;
  }
  return -1;
}

void setLedRak(String namaRak, bool nyala) {
  int rakIdx = getRakIndex(namaRak);
  if (rakIdx == -1) return;
  int slotIdx = getSlotIndex(namaRak, rakIdx);
  if (slotIdx == -1) return;

  statusSlot[rakIdx][slotIdx] = nyala;
  ledArrays[rakIdx][slotIdx] = nyala ? CRGB::Red : CRGB::Black;
}

void showAllStrips() {
  FastLED.show();
}

void clearAllStrips() {
  for (int r = 0; r < 14; r++) {
    fill_solid(ledArrays[r], LED_PER_RAK, CRGB::Black);
  }
}

void turnOffAllLights() {
  Serial.println("Cleanup: Mematikan semua indikator...");
  for (int i = 1; i <= 14; i++) {
    sendWizCommand(getIpForRak(i), false);
    delay(20);
  }
  for (int r = 0; r < 14; r++) {
    for (int s = 0; s < 16; s++) statusSlot[r][s] = false;
  }
  clearAllStrips();
  showAllStrips();
}

void refreshLedState() {
  clearAllStrips();
  for (int r = 0; r < 14; r++) {
    for (int s = 0; s < 16; s++) {
      if (statusSlot[r][s]) ledArrays[r][s] = CRGB::Red;
    }
  }
  showAllStrips();
}

void prosesSatuRak(String namaRak, bool &adaLampuMenyala) {
  namaRak.trim();
  if (namaRak.length() == 0) return;

  Serial.println("- Proses rak: [" + namaRak + "]");

  int noWiz = getWizNumber(namaRak);
  if (noWiz > 0) {
    sendWizCommand(getIpForRak(noWiz), true);
    adaLampuMenyala = true;
  }

  int rakIdx = getRakIndex(namaRak);
  int slotIdx = (rakIdx != -1) ? getSlotIndex(namaRak, rakIdx) : -1;
  Serial.println("  -> rakIdx: " + String(rakIdx) + ", slotIdx: " + String(slotIdx));

  if (rakIdx != -1 && slotIdx != -1) {
    setLedRak(namaRak, true);
    adaLampuMenyala = true;
  } else {
    Serial.println("  -> PERINGATAN: rak [" + namaRak + "] tidak valid!");
  }
}

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

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, PIN_A, GRB>(ledsA, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_B, GRB>(ledsB, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_C, GRB>(ledsC, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_D, GRB>(ledsD, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_E, GRB>(ledsE, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_F, GRB>(ledsF, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_G, GRB>(ledsG, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_H, GRB>(ledsH, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_I, GRB>(ledsI, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_J, GRB>(ledsJ, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_K, GRB>(ledsK, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_L, GRB>(ledsL, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_M, GRB>(ledsM, LED_PER_RAK);
  FastLED.addLeds<WS2812B, PIN_N, GRB>(ledsN, LED_PER_RAK);

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
    WiFi.setSleep(false);
  } else {
    Serial.println("\nGagal connect WiFi saat setup, akan dicoba lagi di loop().");
  }

  udp.begin(wizPort);
}

void loop() {
  unsigned long waktuSekarang = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (waktuSekarang - waktuTerakhirReconnect >= intervalReconnect) {
      waktuTerakhirReconnect = waktuSekarang;
      Serial.println("WiFi terputus, mencoba reconnect...");
      WiFi.reconnect();
    }
  }

  if (statusLampuAktif && (waktuSekarang - waktuRefreshLed >= intervalRefreshLed)) {
    waktuRefreshLed = waktuSekarang;
    refreshLedState();
  }

  if (statusLampuAktif && (waktuSekarang - waktuLampuNyala >= batasWaktuNyala)) {
    turnOffAllLights();
    statusLampuAktif = false;
  }

  if (waktuSekarang - waktuTerakhirCekAPI >= intervalCekAPI || waktuTerakhirCekAPI == 0) {
    waktuTerakhirCekAPI = waktuSekarang;

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;

      if (http.begin(client, apiUrl)) {
        http.setTimeout(10000);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
          String payload = http.getString();
          Serial.println("=== Payload diterima (kode " + String(httpResponseCode) + ") ===");
          Serial.println(payload);

          DynamicJsonDocument doc(8192);
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

              for (JsonVariant v : rakArray) {
                String rakMentah = v.as<String>();
                prosesRakMentah(rakMentah, adaLampuMenyala);
              }

              if (adaLampuMenyala) {
                delay(50);
                refreshLedState();
                waktuLampuNyala = millis();
                waktuRefreshLed = millis();
                statusLampuAktif = true;
              }

              lastOrderId = currentOrderId;
            }
          } else {
            Serial.println("GAGAL parsing JSON! Error: " + String(error.c_str()));
          }
        } else {
          Serial.println("HTTP GET gagal, kode: " + String(httpResponseCode));
        }
        http.end();
      }
    }
  }
}