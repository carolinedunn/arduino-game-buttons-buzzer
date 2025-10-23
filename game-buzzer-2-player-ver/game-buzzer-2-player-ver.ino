/*
  Two-Team Buzzer Game on Arduino Uno
  - D2 (INT0) = Team 1 button (to GND), INPUT_PULLUP
  - D3 (INT1) = Team 2 button (to GND), INPUT_PULLUP
  - D4        = Reset button (to GND), INPUT_PULLUP
  - D9        = Piezo buzzer (+ via 100–220Ω), passive recommended
  - I2C LCD   = A4(SDA), A5(SCL), addr 0x27 (change if needed)

  Features:
  - Start chime when armed (on boot and after reset)
  - First press wins (arrival order via interrupts)
  - Distinct melodies per team
  - Auto-reset after TIMEOUT_MS or manual reset button
  - Debounce both buttons + reset
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//////////////////// CONFIG ////////////////////
const uint8_t PIN_BTN_T1   = 2;   // INT0
const uint8_t PIN_BTN_T2   = 3;   // INT1
const uint8_t PIN_BTN_RST  = 4;   // reset button
const uint8_t PIN_BUZZER   = 9;   // passive piezo

const char* TEAM1_NAME = "Team 1";
const char* TEAM2_NAME = "Team 2";

const unsigned long DEBOUNCE_MS = 30;
const unsigned long TIMEOUT_MS  = 8000; // auto-reset delay

// LCD
const uint8_t  LCD_ADDR = 0x27;  // change to 0x3F if needed
const uint8_t  LCD_COLS = 16;
const uint8_t  LCD_ROWS = 2;
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
////////////////////////////////////////////////

// Volatiles touched in ISRs
volatile bool roundOpen = true;
volatile char winner = 0;               // '1' or '2'
volatile unsigned long lastEdgeT1 = 0;
volatile unsigned long lastEdgeT2 = 0;

// Non-volatile state
unsigned long winAt = 0;
unsigned long lastRstEdge = 0;

//////////////////// Notes (Hz) ////////////////////
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_E6  1319
#define NOTE_G6  1568
////////////////////////////////////////////////////

void toneGap(int freq, int durMs, int gapMs) {
  if (freq > 0) tone(PIN_BUZZER, freq, durMs);
  delay(durMs);
  noTone(PIN_BUZZER);
  if (gapMs > 0) delay(gapMs);
}

// Distinct melodies
void playStartChime() {
  // Simple arming chime
  toneGap(NOTE_C5, 90, 10);
  toneGap(NOTE_E5, 90, 10);
  toneGap(NOTE_G5, 120, 0);
}

void playTeam1Melody() {
  // Ascending & bright
  toneGap(NOTE_E5, 120, 20);
  toneGap(NOTE_G5, 120, 20);
  toneGap(NOTE_C6, 180, 0);
}

void playTeam2Melody() {
  // Descending
  toneGap(NOTE_C6, 120, 20);
  toneGap(NOTE_G5, 120, 20);
  toneGap(NOTE_E5, 180, 0);
}

//////////////////// LCD helpers ////////////////////
// Make n spaces safely
String spaces(uint8_t n) {
  String s;
  s.reserve(n);
  for (uint8_t i = 0; i < n; ++i) s += ' ';
  return s;
}

String center16(const String &txt) {
  String t = txt.length() > 16 ? txt.substring(0, 16) : txt;
  int padTotal = 16 - t.length();
  int padLeft  = padTotal / 2;
  int padRight = padTotal - padLeft;
  return spaces(padLeft) + t + spaces(padRight);
}

void showReady() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(center16("READY"));
  lcd.setCursor(0,1); lcd.print(center16("Press a button"));
}

void showWinner(const char* team) {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(center16(String(team) + " WINS!"));
  lcd.setCursor(0,1); lcd.print(center16("Wait/Reset to play"));
}
/////////////////////////////////////////////////////

// ISRs (keep minimal)
void ISR_Team1() {
  unsigned long now = millis();
  if (now - lastEdgeT1 < DEBOUNCE_MS) return;
  lastEdgeT1 = now;
  if (roundOpen && winner == 0) {
    winner = '1';
    roundOpen = false;
  }
}

void ISR_Team2() {
  unsigned long now = millis();
  if (now - lastEdgeT2 < DEBOUNCE_MS) return;
  lastEdgeT2 = now;
  if (roundOpen && winner == 0) {
    winner = '2';
    roundOpen = false;
  }
}

void armRound() {
  roundOpen = true;
  winner = 0;
  showReady();
  playStartChime();
}

void setup() {
  // Buttons
  pinMode(PIN_BTN_T1, INPUT_PULLUP);
  pinMode(PIN_BTN_T2, INPUT_PULLUP);
  pinMode(PIN_BTN_RST, INPUT_PULLUP);

  // Buzzer
  pinMode(PIN_BUZZER, OUTPUT);
  noTone(PIN_BUZZER);

  // LCD
  lcd.init();
  lcd.backlight();

  // Interrupts on falling edge (pressed → LOW)
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_T1), ISR_Team1, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_T2), ISR_Team2, FALLING);

  armRound();
}

void loop() {
  // Handle winner (edge-triggered via ISR)
  if (winner != 0) {
    // Latch local copy once to avoid repeated actions
    char w = winner;
    winner = 0;            // mark as consumed in main loop
    winAt = millis();

    // Show and sound
    if (w == '1') {
      showWinner(TEAM1_NAME);
      playTeam1Melody();
    } else {
      showWinner(TEAM2_NAME);
      playTeam2Melody();
    }
  }

  // Manual reset button (debounced)
  if (digitalRead(PIN_BTN_RST) == LOW) {
    unsigned long now = millis();
    if (now - lastRstEdge > DEBOUNCE_MS) {
      lastRstEdge = now;
      armRound();
    }
  }

  // Auto-reset after timeout once round is closed
  if (!roundOpen) {
    if (millis() - winAt >= TIMEOUT_MS) {
      armRound();
    }
  }
}
