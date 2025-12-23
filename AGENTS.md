# AGENTS.md - AI Agent Context for 7MHz SSB Transceiver Project

This document provides context for AI coding assistants working on this project.

## Project Overview

This is a **homebrew amateur radio transceiver** project targeting the 7MHz (40-meter) band with SSB (Single Sideband) modulation. The design prioritizes simplicity and uses readily available components.

## Technical Context

### Architecture Type
- **Single-conversion superheterodyne** receiver/transmitter
- **IF frequency**: 9 MHz
- **Local oscillator**: Si5351A DDS (software-controlled)

### Design Philosophy
- Use common, inexpensive ICs (SA612 for all mixing functions)
- Arduino-based control for flexibility
- Self-built crystal ladder filter for IF selectivity
- Class C power amplifier for efficiency

## File Structure and Purpose

### KiCad Schematics (*.kicad_sch)

| File | Purpose | Key Components |
|------|---------|----------------|
| `7mhz_ssb_trx.kicad_sch` | Top-level hierarchy sheet | Sheet symbols linking to sub-sheets |
| `control.kicad_sch` | Digital control system | Arduino Nano, Si5351A, OLED, rotary encoder |
| `mixer_modulator.kicad_sch` | RF signal processing | SA612 x4 (RX mixer, TX mixer, balanced modulator, product detector) |
| `if_filter.kicad_sch` | IF selectivity | 4-element crystal ladder filter (9MHz) |
| `power_amplifier.kicad_sch` | TX power stage | 2SC2314 driver, 2SC1971 final (5W) |
| `bpf_lpf.kicad_sch` | Antenna filters | RX bandpass filter, TX lowpass filter, T/R relay |
| `power_tr.kicad_sch` | Power management | 7809/7805 regulators, relay driver transistor |
| `audio.kicad_sch` | Audio I/O | Mic preamp (2SC1815), AF power amp (LM386) |

### Firmware

| File | Purpose |
|------|---------|
| `firmware/7mhz_ssb_trx.ino` | Main Arduino sketch for VFO control, display, PTT handling |

## Key Design Decisions

### Why SA612 for all mixers?
- Low cost and readily available
- Adequate performance for 7MHz band (not heavily congested)
- Simplifies design (same IC everywhere)
- Built-in oscillator capability (used for carrier in some designs)

### Why 9MHz IF?
- Standard frequency with available crystals
- Good image rejection for 7MHz band (image at 25MHz)
- Well-documented ladder filter designs available

### Why Si5351A?
- Very inexpensive (<$5 for module)
- 3 independent clock outputs (VFO, BFO, spare)
- I2C control from Arduino
- Excellent frequency stability

### Why crystal ladder filter instead of commercial?
- Educational value (understanding filter design)
- Cost effective (a few dollars vs. $20-100 for commercial filters)
- Can be tuned/adjusted for specific bandwidth

## Frequency Plan

```
RX Operation:
  Antenna (7.0-7.2 MHz) -> BPF -> Mixer -> IF Filter -> Detector -> Audio
                                   ^                      ^
                          VFO (16.0-16.2 MHz)      BFO (8.9985 MHz)

TX Operation:
  Mic -> Amp -> Balanced Modulator -> IF Filter -> Mixer -> LPF -> Antenna
                       ^                            ^
                BFO (8.9985 MHz)           VFO (16.0-16.2 MHz)

Frequency Calculation:
  VFO = Display_Freq + IF = 7.xxx MHz + 9.000 MHz = 16.xxx MHz
  BFO = 9.000 MHz - 1.5 kHz = 8.9985 MHz (for LSB)
```

## Common Modifications/Improvements

When asked to modify this design, consider:

1. **Adding AGC**: Insert AGC circuit after IF filter using MC1350 or discrete design
2. **Multi-band operation**: Add band-switched BPF/LPF modules, modify VFO range
3. **USB support**: Add second BFO frequency (9.0015 MHz) or use software switching
4. **Higher power**: Replace final stage with larger transistor (RD16HHF1, etc.)
5. **Better display**: Upgrade to larger OLED or add S-meter
6. **CW mode**: Add sidetone oscillator, keying circuit

## Code Style Guidelines

### Arduino Firmware
- Use `#define` for pin assignments and constants
- Frequency variables as `uint64_t` (Si5351 library requirement)
- Store user settings in EEPROM
- Debounce all mechanical inputs
- Update display only when needed (avoid flicker)

### KiCad Schematics
- Use hierarchical sheets for organization
- Place bypass capacitors near IC power pins
- Label all nets clearly
- Include design notes as text annotations

## Testing Procedures

### Recommended Test Sequence
1. **Power supply**: Verify +12V, +9V, +5V rails
2. **Control system**: Check OLED display, encoder, PTT
3. **Si5351 output**: Verify VFO and BFO frequencies with frequency counter
4. **RX chain**: Inject 9MHz signal, verify audio output
5. **TX chain**: Key transmitter, measure output power and frequency
6. **Filter alignment**: Tune BPF for maximum sensitivity, verify LPF harmonic suppression

### Critical Measurements for Licensing
- Output power (should be <10W for 4th class license)
- Spurious emissions (2nd harmonic, 3rd harmonic)
- Occupied bandwidth (<3kHz for SSB)

## Troubleshooting Hints

| Symptom | Likely Cause |
|---------|--------------|
| No display | I2C address wrong (try 0x3C or 0x3D), wiring issue |
| No VFO output | Si5351 not initialized, I2C communication failure |
| Weak RX | BPF misaligned, IF filter insertion loss high |
| Distorted TX | Balanced modulator carrier suppression poor, PA overdriven |
| Spurious emissions | LPF component values wrong, PA oscillating |

## Dependencies and Tools

### Hardware Development
- KiCad 8.0 (schematic and PCB)
- LTSpice (filter simulation)

### Firmware Development
- Arduino IDE 2.x or PlatformIO
- Libraries: Adafruit_GFX, Adafruit_SSD1306, Si5351, Rotary

### Test Equipment (Recommended)
- Frequency counter
- Oscilloscope
- RF power meter
- Dummy load (50 ohm)
- Signal generator (optional)

## Regulatory Notes

This is an **amateur radio project** requiring:
- Valid amateur radio license (4th class or higher in Japan)
- Station license with equipment registration
- Compliance with spurious emission regulations

The design must pass JARD/TSS guarantee inspection or self-certification with measured data.

## Contact and Support

This project was designed as a homebrew educational project. For questions about amateur radio licensing in Japan, contact JARL or your local radio club.
