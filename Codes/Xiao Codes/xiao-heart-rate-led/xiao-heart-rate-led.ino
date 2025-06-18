#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "MAX30105.h"
#include "heartRate.h"

// === Pin Definitions ===
#define SDA_PIN 6          // D4 on XIAO ESP32-C3
#define SCL_PIN 7          // D5 on XIAO ESP32-C3
#define NEOPIXEL_PIN 21    // D6 on XIAO ESP32-C3
#define NUM_LEDS     8

// === Global Objects ===
Adafruit_NeoPixel ring(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
MAX30105 particleSensor;

// === Heart Rate Variables ===
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  ring.begin();
  ring.clear();
  ring.show();

  Serial.println("Initializing MAX30102...");
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found.");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x2A);
  particleSensor.setPulseAmplitudeGreen(0);
}

void loop() {
  long irValue = particleSensor.getIR();

  if (irValue < 50000) {
    ring.clear();
    ring.show();
    Serial.println("No finger detected");
    delay(200);
    return;
  }

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute > 20 && beatsPerMinute < 255) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte i = 0; i < RATE_SIZE; i++) {
        beatAvg += rates[i];
      }
      beatAvg /= RATE_SIZE;

      Serial.print("Avg BPM: ");
      Serial.println(beatAvg);

      // === LED Color Logic ===
      uint32_t color;
      if (beatAvg < 60) {
        color = ring.Color(255, 0, 0);   // Red
      } else if (beatAvg <= 100) {
        color = ring.Color(0, 255, 0);   // Green
      } else {
        color = ring.Color(255, 0, 0);   // Red
      }

      for (int i = 0; i < NUM_LEDS; i++) {
        ring.setPixelColor(i, color);
      }
      ring.show();
    }
  }

  delay(20);
}
