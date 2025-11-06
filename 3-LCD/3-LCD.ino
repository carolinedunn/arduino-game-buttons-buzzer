// Game Night Buzzer LCD Test Script
// Board: Arduino Uno/Nano (AVR)
// Test 16x2 I2C LCD screen connection to Arduino Uno
// Success = LCD screen display Line 1: Welcome to, Line 2: Game Night

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
