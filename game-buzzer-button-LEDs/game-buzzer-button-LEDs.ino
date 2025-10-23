/*
  6-Player Buzzer Game - Arduino Uno (with per-player LED lighting on win)
  - First press wins (arrival order, debounced).
  - Start chime on arm (boot and reset).
  - Distinct win tones for each team.
  - Winning team's arcade-button LED lights up.
  - Auto-reset after TIMEOUT_MS or manual reset button.

  Buttons (to GND, INPUT_PULLUP):
    Team 1..6: D2, D3, D4, D5, D6, D7
    Reset:     D8 -> GND

  Piezo (passive): D9 -> 100–220Ω -> piezo + ; piezo - -> GND

  LEDs (through 220 Ω to Anode, Cathode -> GND):
    Team 1..6 LEDs: D10, D11, D12, D13, A0, A1

  I2C LCD: A4 (SDA), A5 (SCL), VCC 5V, GND
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ------------- CONFIG -------------
const uint8_t NUM_PLAYERS = 6;

const uint8_t BTN_PINS[NUM_PLAYERS] = {2, 3, 4, 5, 6, 7};
const uint8_t LED_PINS[NUM_PLAYERS] = {10, 11, 12, 13, A0, A1};  // LED pins per team

// If your LEDs are active-low (e.g., common-anode wiring), set this to false.
const bool LED_ACTIVE_HIGH = true;

const char*   TEAM_NAMES[NUM_PLAYERS] = {
  "Team 1", "Team 2", "Team 3", "Team 4", "Team 5", "Team 6"
};

const uint8_t PIN_BTN_RST = 8;
const uint8_t PIN_BUZZER  = 9;

const unsigned long DEBOUNCE_MS = 30;
const unsigned long TIMEOUT_MS  = 8000;   // auto-reset after win (ms)

// LCD
const uint8_t LCD_ADDR = 0x27;  // change to 0x3F if needed
const uint8_t LCD_COLS = 16;
const uint8_t LCD_ROWS = 2;

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
// ----------------------------------

// --------- Notes (Hz) -------------
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_EB5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_G6  1568
// ----------------------------------

// State
bool prevLevel[NUM_PLAYERS];
unsigned long lastEdge[NUM_PLAYERS];
int winnerIndex = -1;
bool roundOpen = true;

unsigned long lastRstEdge = 0;
unsigned long winAt = 0;

// ---------- LED helpers ----------
void ledWrite(uint8_t idx, bool on) {
  if (LED_ACTIVE_HIGH) {
    digitalWrite(LED_PINS[idx], on ? HIGH : LOW);
  } else {
    digitalWrite(LED_PINS[idx], on ? LOW : HIGH);
  }
}

void allLedsOff() {
  for (uint8_t i = 0; i < NUM_PLAYERS; ++i) ledWrite(i, false);
}

void lightWinner(uint8_t idx) {
  allLedsOff();
  ledWrite(idx, true);  // steady ON; change to blink if you prefer
}

// ---------- Sound helpers ----------
void toneGap(int freq, int durMs, int gapMs) {
  if (freq > 0) tone(PIN_BUZZER, freq, durMs);
  delay(durMs);
  noTone(PIN_BUZZER);
  if (gapMs > 0) delay(gapMs);
}

// Start chime when arming
void playStartChime() {
  toneGap(NOTE_C5, 90, 10);
  toneGap(NOTE_E5, 90, 10);
  toneGap(NOTE_G5,120, 0);
}

/* Distinct short win signatures */
void playWinMelody(uint8_t idx) {
  switch (idx) {
    case 0: // Team 1
      toneGap(NOTE_E5,120,20); toneGap(NOTE_G5,120,20); toneGap(NOTE_C6,180,0); break;
    case 1: // Team 2
      toneGap(NOTE_C6,120,20); toneGap(NOTE_G5,120,20); toneGap(NOTE_E5,180,0); break;
    case 2: // Team 3
      toneGap(NOTE_C5,100,15); toneGap(NOTE_E5,100,15); toneGap(NOTE_G5,160,0); break;
    case 3: // Team 4
      toneGap(NOTE_G5,100,15); toneGap(NOTE_E5,100,15); toneGap(NOTE_C5,160,0); break;
    case 4: // Team 5
      toneGap(NOTE_D5,100,15); toneGap(NOTE_F6,100,15); toneGap(NOTE_D6,160,0); break;
    case 5: // Team 6
      toneGap(NOTE_EB5,100,15); toneGap(NOTE_E5,100,15); toneGap(NOTE_G6,160,0); break;
  }
}

// -------- LCD helpers (no String tricks) --------
void printCentered16(uint8_t row, const char* msg) {
  int len = 0;
  while (msg[len] && len < 16) len++;
  int padLeft = (16 - len) / 2;

  lcd.setCursor(0, row);
  for (int i = 0; i < padLeft; ++i) lcd.print(' ');
  for (int i = 0; i < len;     ++i) lcd.print(msg[i]);
  for (int i = padLeft + len; i < 16; ++i) lcd.print(' ');
}

void showReady() {
  lcd.clear();
  printCentered16(0, "READY");
  printCentered16(1, "Press a button");
}

void showWinner(uint8_t idx) {
  char line[17];
  snprintf(line, sizeof(line), "%s WINS!", TEAM_NAMES[idx]); // safely truncated
  lcd.clear();
  printCentered16(0, line);
  printCentered16(1, "Wait/Reset to play");
}
// -----------------------------------------------

void armRound() {
  roundOpen = true;
  winnerIndex = -1;
  allLedsOff();
  showReady();
  playStartChime();
}

void setup() {
  // Buttons
  for (uint8_t i = 0; i < NUM_PLAYERS; ++i) {
    pinMode(BTN_PINS[i], INPUT_PULLUP);
    prevLevel[i] = HIGH;   // idle high (pull-up)
    lastEdge[i] = 0;
  }
  pinMode(PIN_BTN_RST, INPUT_PULLUP);

  // LEDs
  for (uint8_t i = 0; i < NUM_PLAYERS; ++i) {
    pinMode(LED_PINS[i], OUTPUT);
  }
  allLedsOff();

  // Buzzer
  pinMode(PIN_BUZZER, OUTPUT);
  noTone(PIN_BUZZER);

  // LCD
  lcd.init();
  lcd.backlight();

  armRound();
}

void loop() {
  unsigned long now = millis();

  // --- Scan all player buttons for first falling edge ---
  if (roundOpen) {
    for (uint8_t i = 0; i < NUM_PLAYERS; ++i) {
      int level = digitalRead(BTN_PINS[i]);   // HIGH idle, LOW pressed
      if (prevLevel[i] == HIGH && level == LOW) {
        if (now - lastEdge[i] > DEBOUNCE_MS) {
          // First valid press wins
          winnerIndex = i;
          roundOpen = false;
          winAt = now;

          lightWinner(i);      // light the winning button LED
          showWinner(i);       // update LCD
          playWinMelody(i);    // play distinct melody
          break;               // stop scanning after winner
        }
        lastEdge[i] = now;
      }
      prevLevel[i] = level;
    }
  }

  // --- Manual reset button (debounced) ---
  if (digitalRead(PIN_BTN_RST) == LOW) {
    if (now - lastRstEdge > DEBOUNCE_MS) {
      lastRstEdge = now;
      armRound();
    }
  }

  // --- Auto-reset after timeout once closed ---
  if (!roundOpen && (now - winAt >= TIMEOUT_MS)) {
    armRound();
  }
}
