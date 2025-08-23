#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD address (change to your module's address if needed, e.g. 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define integratingTime 15000   // Logging period in milliseconds (15s)
#define oneMinute 60000         // One minute

volatile unsigned long counts = 0;   // variable for GM Tube events
unsigned long cpm = 0;               // variable for CPM
unsigned int multiplier;             // variable for calculation CPM
unsigned long previousMillis;        // time measurement

// --- Averages ---
double avgCounts = 0;
double avgCPM = 0;
double avgUSV = 0;
double sumCPM = 0;
double sumUSV = 0;
double uSv = 0;

// --- 15s snapshot ---
unsigned long count15s = 0;

volatile bool pulseDetected = false;  // flag to signal LED blink
const int ledPin = 9;

// Geiger counter interrupt
void tube_impulse() {
  counts++;  
  pulseDetected = true;
}

void setup() {
  lcd.init();          
  lcd.backlight();     

  multiplier = oneMinute / integratingTime; // CPM calculation factor

  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2), tube_impulse, FALLING);
  pinMode(ledPin, OUTPUT);

// --- Intro screen ---
  String title = "Radiation Monitor                "; 
  // ^ added lots of trailing spaces so it scrolls fully off screen

  lcd.setCursor(0, 1);
  lcd.print("RM-3000"); // stays fixed

  // Scroll manually only on top row
  for (int i = 0; i <= title.length() - 16; i++) {
    lcd.setCursor(0, 0);
    lcd.print(title.substring(i, i + 16)); // window of 16 chars
    delay(500);
  }

  delay(1000);
  lcd.clear(); // clear after intro
}


void loop() {
  unsigned long currentMillis = millis();

  // --- Blink LED if pulse detected ---
  if (pulseDetected) {
    digitalWrite(ledPin, HIGH);
    delay(5);                  
    digitalWrite(ledPin, LOW);
    pulseDetected = false;     
  }

  // --- Every 15s, update readings ---
  if (currentMillis - previousMillis >= integratingTime) {
    avgCounts++;
    previousMillis = currentMillis;

    count15s = counts;            // snapshot for 15s
    cpm = counts * multiplier;    // scale to CPM
    uSv = cpm / 151.0;            // Conversion factor (depends on tube)

    // update running averages
    sumCPM += cpm;
    sumUSV += uSv;
    avgCPM = sumCPM / avgCounts;
    avgUSV = sumUSV / avgCounts;

    // Debug to Serial
    Serial.print("CPM: "); Serial.print(cpm);
    Serial.print("\t uSv/h: "); Serial.print(uSv, 4);
    Serial.print("\t 15s: "); Serial.println(count15s);

    // --- LCD Display ---
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CPM:");
    lcd.print(cpm);

    lcd.setCursor(9, 0);  // right side of first row
    lcd.print("15s:");
    lcd.print(count15s);

    lcd.setCursor(0, 1);
    lcd.print("uSv/h:");
    lcd.print(uSv, 3);

    counts = 0; // Reset for next 15s
  }
}
