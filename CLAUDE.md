# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

7MHz (40-meter band) SSB transceiver for amateur radio. Detailed technical context is documented in [AGENTS.md](AGENTS.md).

## Building the Firmware

### Required Libraries (install via Arduino IDE Library Manager)
- Adafruit GFX Library
- Adafruit SSD1306
- Si5351 by Jason Milldrum NT7S
- Rotary by Ben Buxton

### Compile and Upload
1. Open `firmware/7mhz_ssb_trx/7mhz_ssb_trx.ino` in Arduino IDE
2. Select Board: "Arduino Nano"
3. Select Processor: "ATmega328P" (or "Old Bootloader" for clones)
4. Compile and upload
