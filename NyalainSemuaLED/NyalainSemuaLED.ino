#include <FastLED.h>

// =====================================================
// KONFIGURASI
// =====================================================

#define LEDS_PER_RACK 16
#define BRIGHTNESS 100

// 14 GPIO DATA
#define PIN_1  13
#define PIN_2  14
#define PIN_3  27
#define PIN_4  26
#define PIN_5  25
#define PIN_6  33
#define PIN_7  32
#define PIN_8  4
#define PIN_9  16
#define PIN_10 17
#define PIN_11 5
#define PIN_12 18
#define PIN_13 19
#define PIN_14 21


// =====================================================
// ARRAY LED
// =====================================================

CRGB leds1[LEDS_PER_RACK];
CRGB leds2[LEDS_PER_RACK];
CRGB leds3[LEDS_PER_RACK];
CRGB leds4[LEDS_PER_RACK];
CRGB leds5[LEDS_PER_RACK];
CRGB leds6[LEDS_PER_RACK];
CRGB leds7[LEDS_PER_RACK];
CRGB leds8[LEDS_PER_RACK];
CRGB leds9[LEDS_PER_RACK];
CRGB leds10[LEDS_PER_RACK];
CRGB leds11[LEDS_PER_RACK];
CRGB leds12[LEDS_PER_RACK];
CRGB leds13[LEDS_PER_RACK];
CRGB leds14[LEDS_PER_RACK];


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // 14 jalur WS2812B
  FastLED.addLeds<WS2812B, PIN_1, GRB>(leds1, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_2, GRB>(leds2, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_3, GRB>(leds3, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_4, GRB>(leds4, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_5, GRB>(leds5, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_6, GRB>(leds6, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_7, GRB>(leds7, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_8, GRB>(leds8, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_9, GRB>(leds9, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_10, GRB>(leds10, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_11, GRB>(leds11, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_12, GRB>(leds12, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_13, GRB>(leds13, LEDS_PER_RACK);
  FastLED.addLeds<WS2812B, PIN_14, GRB>(leds14, LEDS_PER_RACK);

  FastLED.setBrightness(BRIGHTNESS);


  // ===================================================
  // SEMUA 224 LED MERAH
  // ===================================================

  fill_solid(leds1,  LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds2,  LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds3,  LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds4,  LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds5,  LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds6,  LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds7,  LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds8,  LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds9,  LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds10, LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds11, LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds12, LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds13, LEDS_PER_RACK, CRGB::Red);
  fill_solid(leds14, LEDS_PER_RACK, CRGB::Red);

  FastLED.show();

  Serial.println("==============================");
  Serial.println("14 JALUR DATA WS2812B");
  Serial.println("SEMUA LED MERAH");
  Serial.println("Total LED = 224");
  Serial.println("==============================");
}


void loop() {
  // Semua LED tetap menyala merah
}