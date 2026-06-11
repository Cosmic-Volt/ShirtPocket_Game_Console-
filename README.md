# ShirtPocket_Game_Console-
DIY ESP8266 handheld game console with OLED display, buzzer, 12 built-in games, music playback, and interactive utilities
PS : I created this projcet well before the announcement of Nintendo Switch 2 so it has the name. 


# ESP8266 OLED Game Console

A handheld game console built using ESP8266, OLED display, buttons and buzzer.

## Features

### Games
- Snake
- Pong
- Dino Runner
- Flappy Square
- Doodle Jump
- Puzzle
- Simon Says

### Music
- Fur Elise
- Pink Panther

### Utilities
- Coin Flipper
- Morse Code Translator
- Fortune Cookie Generator

## Components

| Component | Quantity |
|------------|------------|
| ESP8266 NodeMCU | 1 |
| OLED SSD1306 | 1 |
| Buttons | 4 |
| Passive Buzzer | 1 |
| Battery | 1 |

## Wiring

| OLED | ESP8266 |
|-------|---------|
| SDA | D6 |
| SCL | D7 |

| Button | ESP8266 |
|---------|---------|
| UP | D1 |
| DOWN | D2 |
| LEFT | D4 |
| RIGHT | D3 |

| Buzzer | ESP8266 |
|---------|---------|
| + | D0 |
| - | GND |

## Installation

1. Install Arduino IDE.
2. Install ESP8266 Board Package.
3. Install:
   - Adafruit GFX
   - Adafruit SSD1306
4. Upload code.

## Controls

- UP = Navigate / Jump
- DOWN = Navigate
- LEFT = Select
- RIGHT = Move

## Future Improvements

- EEPROM high scores
- Battery indicator
- SD card support
- MP3 player

## License

MIT License
