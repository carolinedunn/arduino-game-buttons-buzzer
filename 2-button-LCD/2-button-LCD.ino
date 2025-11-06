/*
  Reset Button Test with LCD Screen
  - LCD shows "Ready" / "Press Reset Button" on boot
  - When reset button pressed, LCD shows "Reset Button Pressed"
  - LCD returns to ready message after 5 seconds

  Reset Button: D8 -> GND
  I2C LCD: A4 (SDA), A5 (SCL), VCC 5V, GND
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ------------- CONFIG -------------
const uint8_t PIN_BTN_RST = 8;
const unsigned long DEBOUNCE_MS = 30;
const unsigned long DISPLAY_TIMEOUT_MS = 5000;  // 10 seconds

// LCD
const uint8_t LCD_ADDR = 0x27;  // change to 0x3F if needed
const uint8_t LCD_COLS = 16;
const uint8_t LCD_ROWS = 2;

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
// ----------------------------------

// State
unsigned long lastRstEdge = 0;
unsigned long pressedAt = 0;
bool showingPressed = false;

// -------- LCD helpers --------
void printCentered16(uint8_t row, const char* msg) {
  int len = 0;
  while (msg[len] && len < 16) len++;
  int padLeft = (16 - len) / 2;

  lcd.setCursor(0, row);
  for (int i = 0; i < padLeft; ++i) lcd.print(' ');
  for (int i = 0; i < len; ++i) lcd.print(msg[i]);
  for (int i = padLeft + len; i < 16; ++i) lcd.print(' ');
}

void showReady() {
  lcd.clear();
  printCentered16(0, "Ready");
  printCentered16(1, "Press Button");
  showingPressed = false;
}

void showPressed() {
  lcd.clear();
  printCentered16(0, "Reset Button");
  printCentered16(1, "Pressed");
  showingPressed = true;
  pressedAt = millis();
}
// -----------------------------

void setup() {
  pinMode(PIN_BTN_RST, INPUT_PULLUP);

  // LCD
  lcd.init();
  lcd.backlight();

  showReady();
}

void loop() {
  unsigned long now = millis();

  // --- Reset button detection (debounced) ---
  if (digitalRead(PIN_BTN_RST) == LOW) {
    if (now - lastRstEdge > DEBOUNCE_MS) {
      lastRstEdge = now;
      showPressed();
    }
  }

  // --- Auto-return to ready screen after timeout ---
  if (showingPressed && (millis() - pressedAt >= DISPLAY_TIMEOUT_MS)) {
    showReady();
  }
}
