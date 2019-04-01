#include <Wire.h> // standard I2C library included with the Arduino IDE
#include <math.h> // standard library used for temperature calculations
//#include <LiquidCrystal_I2C.h> // for I2C, you need the new library installed (the link is just below)
#include <LiquidCrystal.h>
#include "OneButton.h"

#define DEBUG_MODE

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
byte customChar[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
int menupin = 7;
int switchpin = 8;
int weldbuttonpin = 6;
int thermpin = 0;
int preweld = 50;
int postprepause = 400;
int weldtimeincrement = 25;
#ifdef DEBUG_MODE
int maxweldpulse = 1000;
#else
int maxweldpulse = 250;
#endif
//int debouncepause = 200;
int weldtime = 0;
int cutofftemp = 80;
int resumetemp = 40;

OneButton menuButton(menupin, true, true);
OneButton weldButton(weldbuttonpin, true, true);

void setup() {
#ifdef DEBUG_MODE
  Serial.begin(9600);
#endif
  pinMode(switchpin, OUTPUT);
  //pinMode(weldbuttonpin, INPUT_PULLUP);
  //pinMode(menupin, INPUT_PULLUP);
  pinMode(thermpin, INPUT);
  maxweldpulse = (maxweldpulse - weldtimeincrement);
  weldtime = weldtimeincrement;
  menuButton.attachClick(menuchange);
  menuButton.attachDuringLongPress(menuchange);
  menuButton.setDebounceTicks(20); // default=50
  menuButton.setClickTicks(100); // default=600
  menuButton.setPressTicks(200); // default=1000
  weldButton.attachClick(weld);
  weldButton.setDebounceTicks(20); // default=50
  weldButton.setClickTicks(100); // default=600
  lcd.createChar(0, customChar);
  lcd.begin(16, 2);
  menudisplay();
}

void loop() {
  if (ThermistorRead(thermpin) >= cutofftemp)  {
    cooldown();
  }
  //if (digitalRead(menupin) == LOW)  {
  //  delay(debouncepause);
  //  menuchange();
  //}
  menuButton.tick();
  //if (digitalRead(weldbuttonpin) == LOW)  {
  //  delay(debouncepause);
  //  weld();
  //}
  weldButton.tick();
  menudisplay();
}

void menuchange() {
  if (weldtime <= maxweldpulse) {
    weldtime = (weldtime + weldtimeincrement);
  }
  else  {
    weldtime = weldtimeincrement;
  }
  menudisplay();
}

void menudisplay()  {
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Weld time|Temp."));
  lcd.setCursor(0, 1);
  lcd.print(weldtime);
  lcd.print(F(" ms  "));
  lcd.setCursor(9, 1);
  lcd.print(F("|"));
  lcd.print((int)ThermistorRead(thermpin));
  lcd.print(F(" "));
  lcd.write((byte)0);
  lcd.print(F("C  "));
  delay (10);
}

void weld() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Welding..."));
  preweld = 0.2 * weldtime;
  digitalWrite(switchpin, HIGH);
  delay(preweld);
  digitalWrite(switchpin, LOW);
  delay(postprepause);
  digitalWrite(switchpin, HIGH);
  delay(weldtime);
  digitalWrite(switchpin, LOW);
  menudisplay();
}

void cooldown() {
  lcd.clear();
  while (ThermistorRead(thermpin) > resumetemp)  {
    lcd.setCursor(0, 0);
    lcd.print(F("Cooling..|Temp."));
    lcd.setCursor(9, 1);
    lcd.print(F("|"));
    lcd.print((int)ThermistorRead(thermpin));
	lcd.print(F(" "));
    lcd.write((byte)0);
    lcd.print(F("C  "));
    delay(10);
  }
  menudisplay();
}

float ThermistorRead(int pin)	{
#define OVERSAMPLES 16
#define EXTRA_BITS 2	// OVERSAMPLES = pow(4, EXTRA_BITS)
  byte i;
  uint16_t oversample = 0;
  float Temp;
 
  for (i=0; i<OVERSAMPLES; i++) {
	oversample += analogRead(pin);
  }
  oversample >>= EXTRA_BITS;
  Temp = ADC2Temp(oversample);
#ifdef DEBUG_MODE
  Serial.println(Temp);
#endif
  return Temp;
}

float ADC2Temp(int RawADC)	{
#define MAX_VIRT_ADC 4092.0
#define ONE_OVER_T0 0.0028316579357214 // 1.0 / (80.0 + 273.15)
  float Temp;
  Temp = 12000.0 / (MAX_VIRT_ADC / RawADC - 1.0);	// R
  Temp = log(Temp / 12540.0);						// ln(R/Ro)
  Temp /= 3950.0;         					    	// 1/B * ln(R/Ro)
  Temp += ONE_OVER_T0;								// + (1/To)
  Temp = 1.0 / Temp;       			   				// Invert
  Temp -= 273.15;           		   				// convert to C
  return Temp;
}