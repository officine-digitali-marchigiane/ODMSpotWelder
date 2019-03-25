#include <Wire.h> // standard I2C library included with the Arduino IDE
#include <math.h> // standard library used for temperature calculations
//#include <LiquidCrystal_I2C.h> // for I2C, you need the new library installed (the link is just below)
#include <LiquidCrystal.h>
#include "OneButton.h"
//#define DEBUG_MODE

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
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
int debouncepause = 200;
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
  lcd.begin(16, 2);
  menudisplay();
}

void loop() {
#ifdef DEBUG_MODE
  Serial.println(ThermistorRead(thermpin));
#endif
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
  lcd.print("Weld time|Temp.");
  lcd.setCursor(0, 1);
  lcd.print(weldtime);
  lcd.print(" ms  ");
  lcd.setCursor(9, 1);
  lcd.print("|");
  lcd.print(ThermistorRead(thermpin));
  lcd.print("C ");
  //delay (100);
}

void weld() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welding...");
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
    lcd.print("Cooling..|Temp.");
    lcd.setCursor(9, 1);
    lcd.print("|");
    lcd.print(ThermistorRead(thermpin));
    lcd.print("C ");
    delay(10);
  }
  menudisplay();
}

float ThermistorRead(int pin)	{
#define NUMSAMPLES 5
  byte i;
  int sample;
  float average = 0.0;
 
  for (i=0; i<NUMSAMPLES; i++) {
	sample = analogRead(pin);
	average += sample;
	delay(10);
  }
  average /= NUMSAMPLES;
  return ADC2Temp2(average);
}

float ADC2Temp(int RawADC) {
  float Temp;
  // Temp = log(100000.0 * ((1023.0 / RawADC - 1)));
  Temp = log(100000.0 / (1023.0 / RawADC - 1)); // for pull-up configuration
  Temp = 1.0 / (0.0008271111 + (0.000208802 + (0.000000080592 * Temp * Temp )) * Temp );
  Temp = Temp - 273.15; // Convert Kelvin to Celsius - Temp = (Temp * 9.0)/ 5.0 + 32.0; for convert Celsius to Fahrenheit
  return Temp;
}

float ADC2Temp1(int RawADC) {
  float Temp;
  // Temp = log(10000.0 * ((1023.0 / RawADC - 1)));
  Temp = log(10000.0 / (1023.0 / RawADC - 1)); // for pull-up configuration
  Temp = 1.0 / (0.001129241 + (0.0002341077 + (0.00000008775468 * Temp * Temp )) * Temp );
  Temp = Temp - 273.15; // Convert Kelvin to Celsius - Temp = (Temp * 9.0)/ 5.0 + 32.0; for convert Celsius to Fahrenheit
  return Temp;
}

float ADC2Temp2(int RawADC)	{
  float Temp;
  Temp = RawADC / (1023.00 - RawADC);	// (R/Ro)
  Temp = log(Temp);            			// ln(R/Ro)
  Temp /= 3950.0;         			    // 1/B * ln(R/Ro)
  Temp += 1.0 / (25.0 + 273.15);		// + (1/To)
  Temp = 1.0 / Temp;       			    // Invert
  Temp -= 273.15;           		    // convert to C
  return Temp;
}