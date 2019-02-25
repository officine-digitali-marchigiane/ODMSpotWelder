#include <Wire.h> // standard I2C library included with the Arduino IDE
#include <math.h> // standard library used for temperature calculations
//#include <LiquidCrystal_I2C.h> // for I2C, you need the new library installed (the link is just below)
#include <LiquidCrystal.h>

#define DEBUG_MODE

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

void setup() {
  pinMode(switchpin, OUTPUT);
  pinMode(weldbuttonpin, INPUT_PULLUP);
  pinMode(menupin, INPUT_PULLUP);
  pinMode(thermpin, INPUT);
  Serial.begin(9600);
  maxweldpulse = (maxweldpulse - weldtimeincrement);
  weldtime = weldtimeincrement;
  lcd.begin(16, 2);
  //  lcd.backlight();
  //  menudisplay();
}

void loop() {
#ifdef DEBUG_MODE
  Serial.println(Thermistor(analogRead(thermpin)));
#endif
  //if (int(Thermistor(analogRead(thermpin))) >= cutofftemp)  {
  //  cooldown();
  //}
  if (digitalRead(menupin) == LOW)  {
    delay(debouncepause);
    menuchange();
  }
  if (digitalRead(weldbuttonpin) == LOW)  {
    delay(debouncepause);
    weld();
  }
  menudisplay(); //aggiunto per aggiornare la temperatura}
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
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Weld Time | Temp");
  lcd.setCursor(0, 1);
  lcd.print(weldtime);
  lcd.print(" ms");
  lcd.setCursor(10, 1);
  lcd.print("|  ");
  lcd.print(int(Thermistor(analogRead(thermpin))));
  lcd.print("C");
  delay (200);
}

void weld() { // This function activates the solid state relay based on your menu selection
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
  while (int(Thermistor(analogRead(thermpin))) > resumetemp)  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cooling...| Temp");
    lcd.setCursor(10, 1);
    lcd.print("|  ");
    lcd.print(int(Thermistor(analogRead(thermpin))));
    lcd.print("C");
    delay(1000);
  }
  menudisplay();
}

double Thermistor(int RawADC) {
  double Temp;
  Temp = log(10000.0 * ((1024.0 / RawADC - 1)));
  //         =log(10000.0/(1024.0/RawADC-1)) // for pull-up configuration
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp )) * Temp );
  Temp = Temp - 273.15;            // Convert Kelvin to Celcius
  // Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
  return Temp;
}
