#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "heartRate.h"

// === Pin Definitions ===
#define SDA_PIN 6            // D4 on XIAO ESP32-C3
#define SCL_PIN 7            // D5 on XIAO ESP32-C3
#define NEOPIXEL_PIN 21      // D6 on XIAO ESP32-C3
#define NUM_LEDS 8

// === OLED Definitions ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === Sensor and NeoPixel ===
MAX30105 particleSensor;
Adafruit_NeoPixel ring(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// === BPM Variables ===
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start I2C for OLED and Sensor
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  // NeoPixel setup
  ring.begin();
  ring.clear();
  ring.show();

  // MAX30102 setup
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found.");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Sensor Error");
    display.display();
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x2A);
  particleSensor.setPulseAmplitudeGreen(0);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Place finger");
  display.display();
}

void loop() {
  long irValue = particleSensor.getIR();

  if (irValue < 50000) {
    ring.clear();
    ring.show();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Place Finger");
    display.display();
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

      // === NeoPixel Color Logic ===
      uint32_t color;
      if(beatAvg<60 && beatAvg>40)
      {
        print("Splash Cold Water");
      }
      else if (beatAvg < 60) {
        color = ring.Color(255, 0, 0);   // Red
      } else if (beatAvg <= 100) {
        print ("The BPM is good");
        color = ring.Color(0, 255, 0);   // Green
      } else {
        print("Take deep breaths");
        color = ring.Color(255, 0, 0);   // Red
      }

      for (int i = 0; i < NUM_LEDS; i++) {
        ring.setPixelColor(i, color);
      }
      ring.show();

      // === OLED Display BPM ===
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.print("BPM: ");
      display.println(beatAvg);
      display.display();
    }
  }

  delay(20);
}
