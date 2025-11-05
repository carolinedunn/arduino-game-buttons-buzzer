# 6-Player Arduino Buzzer Game

A competitive buzzer game for 6 players using Arduino Uno with arcade-style buttons, LED feedback, distinct win tones, and an LCD display.

## Features

- **6 independent player buttons** with arcade-style feel
- **First-press-wins** logic with debouncing
- **Individual LED lighting** for each player's button (lights up on win)
- **Distinct win melodies** for each team
- **16x2 I2C LCD display** showing game status
- **Start chime** when the game is ready
- **Auto-reset** after 8 seconds or manual reset button
- **3D printable enclosure** (files included)

## Materials

- [Arduino Uno kit](https://amzn.to/4nzzP0P)
- [Arcade style buttons](https://amzn.to/47JRmOd)
- [Breadboard](https://amzn.to/4nBJFiM)
- [Piezo buzzer](https://amzn.to/43QuStq)
- [Jumper wires](https://amzn.to/4hDTRpy)
- [Spade connectors](https://amzn.to/43UcBLQ)
- [16x2 I2C LCD display](https://amzn.to/3Llyiy8)

Material links provided are Amazon Affiliate links, which means I may earn a small commission at no extra cost to you.

## Wiring

| Component | Pin(s) | Notes |
|-----------|--------|-------|
| Team 1 Button | D2 | INPUT_PULLUP to GND |
| Team 1 LED | D10 | Through 220Ω to Anode, Cathode to GND |
| Team 2 Button | D3 | INPUT_PULLUP to GND |
| Team 2 LED | D11 | Through 220Ω to Anode, Cathode to GND |
| Team 3 Button | D4 | INPUT_PULLUP to GND |
| Team 3 LED | D12 | Through 220Ω to Anode, Cathode to GND |
| Team 4 Button | D5 | INPUT_PULLUP to GND |
| Team 4 LED | D13 | Through 220Ω to Anode, Cathode to GND |
| Team 5 Button | D6 | INPUT_PULLUP to GND |
| Team 5 LED | A0 | Through 220Ω to Anode, Cathode to GND |
| Team 6 Button | D7 | INPUT_PULLUP to GND |
| Team 6 LED | A1 | Through 220Ω to Anode, Cathode to GND |
| Reset Button | D8 | INPUT_PULLUP to GND |
| Piezo Buzzer | D9 | Through 100-220Ω resistor to piezo +, piezo - to GND |
| LCD SDA | A4 | I2C Data |
| LCD SCL | A5 | I2C Clock |
| LCD VCC | 5V | Power |
| LCD GND | GND | Ground |

## Installation

1. Install the required library:
   - `LiquidCrystal_I2C` (available in Arduino Library Manager)

2. Clone this repository or download the `.ino` file

3. Open the sketch in Arduino IDE

4. Verify the LCD I2C address (default is `0x27`, some displays use `0x3F`)

5. Upload to your Arduino Uno

## 3D Printing

Download the enclosure files from [Thingiverse](https://www.thingiverse.com/thing:7183224)

## How to Play

1. Power on the Arduino - you'll hear a start chime and see "READY" on the LCD
2. First player to press their button wins
3. The winning player's LED lights up and a unique melody plays
4. LCD displays which team won
5. Game auto-resets after 8 seconds, or press the reset button to play again

## Configuration

You can customize these settings in the code:
- `DEBOUNCE_MS` - Button debounce time (default: 30ms)
- `TIMEOUT_MS` - Auto-reset delay (default: 8000ms)
- `LED_ACTIVE_HIGH` - Set to `false` if using common-anode LED wiring
- `LCD_ADDR` - I2C address of your LCD (0x27 or 0x3F)
- `TEAM_NAMES` - Customize team names

