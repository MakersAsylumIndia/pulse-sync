#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "MAX30105.h"
#include "heartRate.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// OLED I2C (Wire)
#define SDA_OLED 21
#define SCL_OLED 22

// MAX30102 I2C (Wire1)
#define SDA_SENSOR 18
#define SCL_SENSOR 19

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;

const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(115200);

  // Initialize OLED I2C
  Wire.begin(SDA_OLED, SCL_OLED);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found!"));
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 30);
  display.println("Initializing...");
  display.display();

  // Initialize MAX30102 I2C on Wire1
  Wire.begin(SDA_SENSOR, SCL_SENSOR);
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found. Check wiring.");
    display.clearDisplay();
    display.setCursor(10, 30);
    display.println("Sensor not found!");
    display.display();
    while (1);
  }

  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A); 
  particleSensor.setPulseAmplitudeGreen(0);  
}

void loop() {
  long irValue = particleSensor.getIR();

  if (irValue < 50000) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH - 90) / 2, SCREEN_HEIGHT / 2 - 4);
    display.println("Wear the wrist band...");
    display.display();
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

      // Display BPM
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setTextSize(2);
      int xPos = (SCREEN_WIDTH - 6 * 6) / 2; // Center for "XXX BPM"
      display.setCursor(xPos, SCREEN_HEIGHT / 2 - 8);
      display.print((int)beatsPerMinute);
      display.print(" BPM");
      display.display();
    }
  }

  delay(20); // Fast refresh
}
