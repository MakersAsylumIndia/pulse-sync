#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_NeoPixel.h>

// Screen & NeoPixel setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define LED_PIN    13
#define NUM_LEDS   8

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;
Adafruit_NeoPixel ring(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Heart rate variables
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(115200);

  // OLED I2C (Wire)
#define SDA_OLED 18
#define SCL_OLED 19

// MAX30102 I2C (Wire1)
#define SDA_SENSOR 21
#define SCL_SENSOR 22
  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (1);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 30);
  display.println("Initializing...");
  display.display();

  // MAX30102 init
  if (!particleSensor.begin(Wire1, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found");
    display.clearDisplay();
    display.setCursor(10, 30);
    display.println("Sensor not found!");
    display.display();
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0); // turn off green

  // NeoPixel init
  ring.begin();
  ring.show();
}

void loop() {
  long irValue = particleSensor.getIR();

  if (irValue < 50000) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20, 30);
    display.println("Wear wrist band");
    display.display();

    for (int i = 0; i < NUM_LEDS; i++) {
      ring.setPixelColor(i, 0);
    }
    ring.show();

    delay(100);
    return;
  }

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
      }
      beatAvg /= RATE_SIZE;

      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(20, 24);
      display.print((int)beatsPerMinute);
      display.print(" BPM");
      display.display();

      // LED color feedback
      uint32_t color;
      if (beatsPerMinute < 60) {
        color = ring.Color(150, 0, 0);    // Red
      } else if (beatsPerMinute < 100) {
        color = ring.Color(0, 150, 0);    // Green
      } else {
        color = ring.Color(150, 0, 0);    // Red
      }

      for (int i = 0; i < NUM_LEDS; i++) {
        ring.setPixelColor(i, color);
      }
      ring.show();
    }
  }

  delay(20);
}
