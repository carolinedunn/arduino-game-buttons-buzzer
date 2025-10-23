// Two-player game timer: LCD (I2C) + LEDs (G/Y/R) + buzzer
// Board: Arduino Uno/Nano (AVR)

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- LCD ----------
const uint8_t LCD_ADDR = 0x27;     // change to 0x3F if your module uses that
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

void setup() {
  // LCD
  lcd.init();
  lcd.backlight();
}

void loop() {
      // LCD message
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("Welcome to");
      lcd.setCursor(0,1); lcd.print("Game Night");
      delay(5000);
    }
