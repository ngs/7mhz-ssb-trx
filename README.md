# 7MHz SSB Transceiver

A homebrew single-conversion SSB transceiver for the 40-meter amateur radio band.

## Specifications

| Parameter | Value |
|-----------|-------|
| Frequency Range | 7.000 - 7.200 MHz |
| Mode | LSB (Lower Sideband) |
| Output Power | 5W |
| IF Frequency | 9 MHz |
| VFO | Si5351A DDS (16.0 - 16.2 MHz) |
| BFO | 8.9985 MHz (fixed) |
| Supply Voltage | 13.8V DC |
| Current (RX) | ~100 mA |
| Current (TX) | ~1 A |

## Block Diagram

```
                          7MHz SSB TRANSCEIVER

    [ANT] <---> [T/R Relay] <---> [BPF] <---> [RX Mixer] ---> [IF Filter] ---> [Product Det] ---> [AF Amp] ---> [Speaker]
                    |                            ^                                   ^
                    |                            |                                   |
                    +---> [LPF] <--- [PA] <--- [TX Mixer] <--- [IF Filter] <--- [Bal Mod] <--- [Mic Amp] <--- [Mic]
                                                 ^                                   ^
                                                 |                                   |
                                            [Si5351 VFO]                       [Si5351 BFO]
                                                 ^
                                                 |
                                           [Arduino Nano]
                                                 |
                                       [OLED] [Encoder] [PTT]
```

## Hardware Architecture

### Control System
- **MCU**: Arduino Nano (ATmega328P)
- **DDS**: Si5351A module (I2C, 3 clock outputs)
- **Display**: 128x64 OLED (SSD1306, I2C)
- **Input**: Rotary encoder with push switch, PTT button

### RF Section
- **Mixer/Modulator/Detector**: SA612 (NE612) x 4
  - U4: RX Mixer (7MHz + 16MHz VFO -> 9MHz IF)
  - U5: Balanced Modulator (Audio + BFO -> 9MHz DSB)
  - U6: TX Mixer (9MHz IF + VFO -> 7MHz RF)
  - U7: Product Detector (9MHz IF + BFO -> Audio)

### IF Filter
- 4-element crystal ladder filter
- Center frequency: 9.000 MHz
- Bandwidth: ~2.4 kHz
- Crystals: HC-49/S (matched set)

### Power Amplifier
- Driver: 2SC2314 (or 2SC1815)
- Final: 2SC1971 (TO-220, heatsink required)
- Output: 4-5W @ 13.8V

### Filters
- **RX BPF**: Double-tuned LC filter (7 MHz)
- **TX LPF**: 7th order Chebyshev (fc = 7.3 MHz)
  - 2nd harmonic (14 MHz): > -40 dB
  - 3rd harmonic (21 MHz): > -60 dB

### Audio
- **Mic Amp**: 2SC1815 common emitter (~20 dB gain)
- **AF Amp**: LM386N-1 (~0.5W @ 8 ohm)

## Project Structure

```
7mhz_ssb_trx/
├── 7mhz_ssb_trx.kicad_pro      # KiCad project file
├── 7mhz_ssb_trx.kicad_sch      # Top-level schematic (hierarchy)
├── control.kicad_sch           # Arduino + Si5351 + OLED
├── mixer_modulator.kicad_sch   # SA612 mixer/modulator/detector
├── if_filter.kicad_sch         # 9MHz crystal ladder filter
├── power_amplifier.kicad_sch   # Driver + Final amplifier
├── bpf_lpf.kicad_sch           # BPF + LPF + T/R relay
├── power_tr.kicad_sch          # Power supply + relay driver
├── audio.kicad_sch             # Mic amp + AF power amp
├── firmware/
│   └── 7mhz_ssb_trx.ino        # Arduino sketch
├── gerber/                     # Gerber files for PCB fabrication
├── docs/                       # Documentation
├── simulation/                 # LTSpice simulations
├── README.md                   # This file
└── AGENTS.md                   # AI agent context
```

## Building the Firmware

### Required Libraries

Install via Arduino IDE Library Manager:

- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `Si5351` by Jason Milldrum NT7S
- `Rotary` by Ben Buxton

### Compilation

1. Open `firmware/7mhz_ssb_trx.ino` in Arduino IDE
2. Select Board: "Arduino Nano"
3. Select Processor: "ATmega328P" (or "Old Bootloader" for clones)
4. Compile and upload

### Pin Assignment

| Pin | Function |
|-----|----------|
| D2 | Encoder A |
| D3 | Encoder B |
| D4 | PTT Input (active low) |
| D5 | Encoder Switch |
| D6 | TX Enable Output |
| A4 | SDA (I2C) |
| A5 | SCL (I2C) |

## PCB Fabrication

### Generating Gerber Files

1. Open `7mhz_ssb_trx.kicad_pcb` in KiCad
2. File -> Plot -> Select "Gerber" format
3. Plot layers: F.Cu, B.Cu, F.SilkS, B.SilkS, F.Mask, B.Mask, Edge.Cuts
4. Generate drill files (.drl)
5. Zip all files in `gerber/` folder

### PCBWay Order Settings

- Layers: 2
- Dimensions: 150mm x 100mm (recommended)
- PCB Thickness: 1.6mm
- Surface Finish: HASL (lead-free)
- Copper Weight: 1oz

## Bill of Materials (Summary)

### ICs
| Part | Quantity | Notes |
|------|----------|-------|
| SA612AN / NE612 | 4 | Mixer IC |
| LM386N-1 | 1 | AF power amp |
| 7809 | 1 | 9V regulator |
| 7805 | 1 | 5V regulator |
| Si5351A module | 1 | DDS |
| Arduino Nano | 1 | MCU |

### Transistors
| Part | Quantity | Notes |
|------|----------|-------|
| 2SC1815 | 4 | General purpose NPN |
| 2SC2314 | 1 | Driver |
| 2SC1971 | 1 | PA Final (TO-220) |

### Crystals & Inductors
| Part | Quantity | Notes |
|------|----------|-------|
| 9MHz HC-49/S | 10 | For selection (need 4) |
| T50-2 (red) | 5 | Toroidal cores |

### Modules & Connectors
| Part | Quantity |
|------|----------|
| OLED 128x64 I2C | 1 |
| Rotary encoder EC11 | 1 |
| DPDT relay 12V | 1 |
| BNC connector | 1 |
| 3.5mm jack | 2 |
| DC jack 2.1mm | 1 |

## Regulatory Compliance (Japan)

This transceiver is designed for amateur radio use under Japanese regulations.

### Requirements for Operation
1. **License**: 4th class (or higher) amateur radio license
2. **Station License**: Apply through JARD or TSS
3. **Self-built equipment**: Submit block diagram and specifications
4. **Spurious emissions**: Must comply with regulations

### JARD/TSS Guarantee
For self-built transceivers, obtain guarantee certification:
- Submit: Block diagram, specifications, measured data
- Fee: ~4,000-5,000 JPY

## References

- ARRL Handbook
- RSGB Radio Communication Handbook
- "Experimental Methods in RF Design" (ARRL)
- Various JA homebrew projects (CQ ham radio, etc.)

## License

This project is released under the MIT License.

## Contributing

Contributions are welcome! Please open an issue or pull request.

## Changelog

### v1.0 (2024-12-24)
- Initial design
- KiCad schematics created
- Arduino firmware implemented
