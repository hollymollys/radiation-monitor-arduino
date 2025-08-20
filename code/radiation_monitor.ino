#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD address (change to your module's address if needed, e.g. 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define integratingTime 15000  // Logging period in milliseconds
#define oneMinute 60000        // One minute

unsigned long counts = 0;     // variable for GM Tube events
unsigned long cpm = 0;        // variable for CPM
unsigned int multiplier;      // variable for calculation CPM
unsigned long previousMillis; // time measurement
double avgCounts = 0;
double avgCPM = 0;
double avgUSV = 0;
double sumCPM = 0;
double sumUSV = 0;
double uSv = 0;

const int ledPin = 9;

// Geiger counter interrupt
void tube_impulse() {
  counts++;
}

void setup() {
  lcd.init();          // Initialize LCD
  lcd.backlight();     // Turn on backlight

  multiplier = oneMinute / integratingTime; // CPM calculation factor

  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2), tube_impulse, FALLING);
  pinMode(ledPin, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Radiation Monitor");
  delay(1500);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  // Every integratingTime ms, update readings
  if (currentMillis - previousMillis >= integratingTime) {
    avgCounts++;
    previousMillis = currentMillis;

    cpm = counts * multiplier;   // Convert counts to CPM
    uSv = cpm / 151.0;           // Conversion factor

    sumCPM += cpm;
    sumUSV += uSv;

    avgCPM = sumCPM / avgCounts;
    avgUSV = sumUSV / avgCounts;

    // Print to Serial Monitor
    Serial.print("CPM: ");
    Serial.print(cpm);
    Serial.print("\t uSv/h: ");
    Serial.println(uSv, 4);

    // Display on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CPM:");
    lcd.print(cpm);
    lcd.setCursor(0, 1);
    lcd.print("uSv/h:");
    lcd.print(uSv, 4);

    counts = 0; // Reset for next period
  }
}
