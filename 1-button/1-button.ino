// Two-player game timer: LCD (I2C) + LEDs (G/Y/R) + buzzer
// Board: Arduino Uno/Nano (AVR)

#include <Arduino.h>
#include <Wire.h>

// ---------- Pins ----------
const uint8_t BTN_PIN = 2;         // interrupt-capable pin on Uno/Nano

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void setup() {
    // Button + ISR
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off
}

void loop() {
    // read the state of the pushbutton value:
  buttonState = digitalRead(BTN_PIN);

    // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {
    // turn LEDs on:
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else {
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off
  }
}
