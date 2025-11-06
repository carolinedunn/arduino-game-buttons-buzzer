/*
  Reset Button Test with LCD Screen and Sound
  - LCD shows "Ready" / "Press Reset Button" on boot
  - When reset button pressed, LCD shows "Reset Button Pressed" and plays sound
  - LCD returns to ready message after 10 seconds

  Reset Button: D8 -> GND
  Buzzer: D9
  I2C LCD: A4 (SDA), A5 (SCL), VCC 5V, GND
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ------------- CONFIG -------------
const uint8_t PIN_BTN_RST = 8;
const uint8_t PIN_BUZZER = 9;
const unsigned long DEBOUNCE_MS = 30;
const unsigned long DISPLAY_TIMEOUT_MS = 10000;  // 10 seconds

// LCD
const uint8_t LCD_ADDR = 0x27;  // change to 0x3F if needed
const uint8_t LCD_COLS = 16;
const uint8_t LCD_ROWS = 2;

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

// --------- Notes (Hz) -------------
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
// ----------------------------------

// State
unsigned long lastRstEdge = 0;
unsigned long pressedAt = 0;
bool showingPressed = false;

// ---------- Sound helpers ----------
void toneGap(int freq, int durMs, int gapMs) {
  if (freq > 0) tone(PIN_BUZZER, freq, durMs);
  delay(durMs);
  noTone(PIN_BUZZER);
  if (gapMs > 0) delay(gapMs);
}

// Start chime when reset pressed
void playStartChime() {
  toneGap(NOTE_C5, 90, 10);
  toneGap(NOTE_E5, 90, 10);
  toneGap(NOTE_G5, 120, 0);
}

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
  printCentered16(1, "Press Reset Button");
  showingPressed = false;
}

void showPressed() {
  lcd.clear();
  printCentered16(0, "Reset Button");
  printCentered16(1, "Pressed");
  showingPressed = true;
  pressedAt = millis();
  playStartChime();  // Play sound when reset pressed
}
// -----------------------------

void setup() {
  pinMode(PIN_BTN_RST, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  noTone(PIN_BUZZER);

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
