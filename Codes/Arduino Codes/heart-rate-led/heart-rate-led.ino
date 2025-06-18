#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "MAX30105.h"
#include "heartRate.h"

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// MAX30102 setup
#define SDA_SENSOR 18
#define SCL_SENSOR 19
MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;
unsigned long lastValidTime = 0;
bool displayCleared = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_SENSOR, SCL_SENSOR);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 25);
  display.print("Place Finger");
  display.display();

  // Sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found");
    display.clearDisplay();
    display.setCursor(10, 25);
    display.print("Sensor Error");
    display.display();
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A); // Turn on Red LED
  particleSensor.setPulseAmplitudeGreen(0);  // Turn off Green LED
}

void loop() {
  long irValue = particleSensor.getIR();
  unsigned long now = millis();

  if (irValue > 50000) {
    displayCleared = false;

    if (checkForBeat(irValue)) {
      long delta = now - lastBeat;
      lastBeat = now;

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute > 20 && beatsPerMinute < 255) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
        beatAvg /= RATE_SIZE;

        Serial.print("BPM: ");
        Serial.println(beatAvg);

        // Display BPM
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(35, 25);
        display.print(beatAvg);
        display.print(" BPM");
        display.display();

        lastValidTime = now;
      }
    }

    // Clear display if idle too long
    if ((now - lastValidTime > 2000) && !displayCleared) {
      display.clearDisplay();
      display.display();
      displayCleared = true;
    }

  } else {
    // No finger
    if (!displayCleared && (now - lastValidTime > 1000)) {
      display.clearDisplay();
      display.display();
      displayCleared = true;
    }
  }

  delay(20);
}
