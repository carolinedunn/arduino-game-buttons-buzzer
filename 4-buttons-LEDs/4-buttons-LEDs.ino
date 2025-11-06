/*
  Arcade Button LED Toggle Test
  - LCD shows "Press an arcade button" on startup
  - When any arcade button is pressed, its LED toggles on/off
  - LCD displays "Team X LED ON" or "Team X LED OFF"
  - Returns to ready message after 10 seconds (unless reset pressed)
  - Reset button: press to show "Reset button pressed" - keeps buttons active, no timeout

  Arcade Buttons: D2-D7 -> GND
  Button LEDs: D10-D13, A0-A1 (6 LEDs total)
  Reset Button: D8 -> GND
  Buzzer: D9
  I2C LCD: A4 (SDA), A5 (SCL), VCC 5V, GND
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ------------- CONFIG -------------
const uint8_t NUM_PLAYERS = 6;
const uint8_t BTN_PINS[NUM_PLAYERS] = {2, 3, 4, 5, 6, 7};
const uint8_t LED_PINS[NUM_PLAYERS] = {10, 11, 12, 13, A0, A1};
const uint8_t PIN_BTN_RST = 8;
const uint8_t PIN_BUZZER = 9;
const unsigned long DEBOUNCE_MS = 50;
const unsigned long DISPLAY_TIMEOUT_MS = 10000;  // 10 seconds

// LCD
const uint8_t LCD_ADDR = 0x27;  // change to 0x3F if needed
const uint8_t LCD_COLS = 16;
const uint8_t LCD_ROWS = 2;

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

const char* TEAM_NAMES[NUM_PLAYERS] = {
  "Team 1", "Team 2", "Team 3", "Team 4", "Team 5", "Team 6"
};

// ----------------------------------

// State
int prevLevel[NUM_PLAYERS];
unsigned long lastEdge[NUM_PLAYERS];
bool ledState[NUM_PLAYERS];
unsigned long lastRstEdge = 0;
unsigned long lastActionTime = 0;
bool showingStatus = false;
bool resetPressed = false;

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
  printCentered16(0, "Press an");
  printCentered16(1, "arcade button");
  showingStatus = false;
  resetPressed = false;
}

void showTeamStatus(uint8_t teamIndex, bool isOn) {
  lcd.clear();
  char line1[17];
  char line2[17];
  snprintf(line1, sizeof(line1), "%s LED", TEAM_NAMES[teamIndex]);
  snprintf(line2, sizeof(line2), "%s", isOn ? "ON" : "OFF");
  
  printCentered16(0, line1);
  printCentered16(1, line2);
  showingStatus = true;
  resetPressed = false;
  lastActionTime = millis();
}

void showResetPressed() {
  lcd.clear();
  printCentered16(0, "Reset button");
  printCentered16(1, "pressed");
  showingStatus = true;
  resetPressed = true;
  lastActionTime = millis();
}
// -----------------------------

void setup() {
  // Buttons
  for (uint8_t i = 0; i < NUM_PLAYERS; ++i) {
    pinMode(BTN_PINS[i], INPUT_PULLUP);
    prevLevel[i] = HIGH;
    lastEdge[i] = 0;
  }
  pinMode(PIN_BTN_RST, INPUT_PULLUP);

  // LEDs - all start OFF
  for (uint8_t i = 0; i < NUM_PLAYERS; ++i) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
    ledState[i] = false;
  }

  // Buzzer
  pinMode(PIN_BUZZER, OUTPUT);
  noTone(PIN_BUZZER);

  // LCD
  lcd.init();
  lcd.backlight();

  showReady();
}

void loop() {
  unsigned long now = millis();

  // --- Scan all arcade buttons ---
  for (uint8_t i = 0; i < NUM_PLAYERS; ++i) {
    int level = digitalRead(BTN_PINS[i]);
    if (prevLevel[i] == HIGH && level == LOW) {
      if (now - lastEdge[i] > DEBOUNCE_MS) {
        // Toggle LED
        ledState[i] = !ledState[i];
        digitalWrite(LED_PINS[i], ledState[i] ? HIGH : LOW);
        
        // Update LCD
        showTeamStatus(i, ledState[i]);
        
        lastEdge[i] = now;
      }
    }
    prevLevel[i] = level;
  }

  // --- Reset button detection ---
  if (digitalRead(PIN_BTN_RST) == LOW) {
    if (now - lastRstEdge > DEBOUNCE_MS) {
      lastRstEdge = now;
      showResetPressed();
    }
  }

  // --- Auto-return to ready screen after timeout (not if reset pressed) ---
  if (showingStatus && !resetPressed && (now - lastActionTime >= DISPLAY_TIMEOUT_MS)) {
    showReady();
  }
}
