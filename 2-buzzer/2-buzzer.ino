// Two-player game timer: LCD (I2C) + LEDs (G/Y/R) + buzzer
// Board: Arduino Uno/Nano (AVR)

#include <Arduino.h>
#include <Wire.h>

// ---------- Pins ----------
const uint8_t BTN_PIN = 2;         // interrupt-capable pin on Uno/Nano
const uint8_t LED_G   = 5;
const uint8_t LED_Y   = 6;
const uint8_t LED_R   = 7;
const uint8_t BUZZER  = 9;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

// --- Helpers ---
void beep(int freq, int ms, int pause) {
  tone(BUZZER, freq, ms);
  delay(ms + pause);
}

void setup() {
    // Button + ISR
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  // LEDs + buzzer
  pinMode(LED_G, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_Y, LOW);
  digitalWrite(LED_R, LOW);
}

void loop() {
    // read the state of the pushbutton value:
  buttonState = digitalRead(BTN_PIN);

    // check if the pushbutton is pressed. If it is, the buttonState is LOW:
  if (buttonState == LOW) {
    // turn LEDs on:
  
  digitalWrite(LED_G, HIGH); 
  digitalWrite(LED_Y, HIGH); 
  digitalWrite(LED_R, HIGH); 
  beep(800, 80, 70); 
  } else {
    // turn LED off:
  digitalWrite(LED_G, LOW); 
  digitalWrite(LED_Y, LOW); 
  digitalWrite(LED_R, LOW); 
  }
}
