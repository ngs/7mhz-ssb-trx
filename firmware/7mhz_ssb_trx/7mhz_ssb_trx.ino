/*
 * 7MHz SSB Transceiver Controller
 *
 * Hardware:
 *   - Arduino Nano
 *   - Si5351A DDS Module (I2C)
 *   - 128x64 OLED Display SSD1306 (I2C)
 *   - Rotary Encoder EC11 with push switch
 *
 * Pin Assignment:
 *   D2  - Encoder A
 *   D3  - Encoder B
 *   D4  - PTT Input (Active Low)
 *   D5  - Encoder Switch
 *   D6  - TX Enable Output
 *   A4  - SDA (I2C)
 *   A5  - SCL (I2C)
 *
 * Frequency Plan:
 *   IF = 9.000 MHz
 *   BFO = 8.9985 MHz (LSB, -1.5kHz offset)
 *   VFO = RX_Freq + IF = 16.0 - 16.2 MHz
 *
 * License: MIT
 * Author: Homebrew Project
 * Date: 2024-12-24
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <si5351.h>
#include <Rotary.h>
#include <EEPROM.h>

// =============================================================================
// Pin Definitions
// =============================================================================
#define PIN_ENC_A       2
#define PIN_ENC_B       3
#define PIN_PTT         4
#define PIN_ENC_SW      5
#define PIN_TX_EN       6

// =============================================================================
// Frequency Constants
// =============================================================================
#define IF_FREQ         9000000ULL    // 9.000 MHz
#define BFO_FREQ        8998500ULL    // 8.9985 MHz (LSB)
#define FREQ_MIN        7000000ULL    // 7.000 MHz
#define FREQ_MAX        7200000ULL    // 7.200 MHz
#define FREQ_DEFAULT    7100000ULL    // 7.100 MHz

// Step sizes
#define STEP_10HZ       10ULL
#define STEP_100HZ      100ULL
#define STEP_1KHZ       1000ULL
#define STEP_10KHZ      10000ULL

// EEPROM addresses
#define EEPROM_FREQ_ADDR    0
#define EEPROM_STEP_ADDR    4
#define EEPROM_MAGIC_ADDR   8
#define EEPROM_MAGIC_VALUE  0xAA55

// =============================================================================
// Display Configuration
// =============================================================================
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define OLED_ADDRESS    0x3C

// =============================================================================
// Global Objects
// =============================================================================
Si5351 si5351;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Rotary encoder = Rotary(PIN_ENC_A, PIN_ENC_B);

// =============================================================================
// Global Variables
// =============================================================================
uint64_t frequency = FREQ_DEFAULT;
uint64_t step_size = STEP_1KHZ;
bool tx_mode = false;
bool display_update_needed = true;

// Step size index for cycling
const uint64_t step_sizes[] = {STEP_10HZ, STEP_100HZ, STEP_1KHZ, STEP_10KHZ};
const uint8_t num_steps = sizeof(step_sizes) / sizeof(step_sizes[0]);
uint8_t step_index = 2;  // Default to 1kHz

// Debounce
unsigned long last_enc_sw_time = 0;
unsigned long last_ptt_time = 0;
const unsigned long debounce_delay = 50;

// =============================================================================
// Function Prototypes
// =============================================================================
void setupSi5351();
void updateVFO();
void updateDisplay();
void handleEncoder();
void handlePTT();
void handleEncoderSwitch();
void saveSettings();
void loadSettings();
void drawFrequency();
void drawStatus();
void drawStepIndicator();

// =============================================================================
// Setup
// =============================================================================
void setup() {
  // Pin setup
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_PTT, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);
  pinMode(PIN_TX_EN, OUTPUT);

  digitalWrite(PIN_TX_EN, LOW);  // Start in RX mode

  // Serial for debugging
  Serial.begin(115200);
  Serial.println(F("7MHz SSB Transceiver"));
  Serial.println(F("Initializing..."));

  // I2C
  Wire.begin();

  // Load saved settings
  loadSettings();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);  // Halt if display fails
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();

  // Initialize Si5351
  setupSi5351();

  // Initial display update
  updateDisplay();

  Serial.println(F("Ready!"));
}

// =============================================================================
// Main Loop
// =============================================================================
void loop() {
  handleEncoder();
  handlePTT();
  handleEncoderSwitch();

  if (display_update_needed) {
    updateDisplay();
    display_update_needed = false;
  }
}

// =============================================================================
// Si5351 Setup
// =============================================================================
void setupSi5351() {
  bool i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);

  if (!i2c_found) {
    Serial.println(F("Si5351 not found!"));
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Si5351 ERROR!"));
    display.display();
    while (1);
  }

  // Set drive strength
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);  // VFO
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_4MA);  // BFO
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA);  // Spare

  // Set initial frequencies
  updateVFO();
  si5351.set_freq(BFO_FREQ * 100ULL, SI5351_CLK1);

  // Disable CLK2 (spare)
  si5351.output_enable(SI5351_CLK2, 0);

  Serial.println(F("Si5351 initialized"));
}

// =============================================================================
// Update VFO Frequency
// =============================================================================
void updateVFO() {
  uint64_t vfo_freq = frequency + IF_FREQ;
  si5351.set_freq(vfo_freq * 100ULL, SI5351_CLK0);

  Serial.print(F("VFO: "));
  Serial.print((uint32_t)(vfo_freq / 1000));
  Serial.println(F(" kHz"));
}

// =============================================================================
// Handle Rotary Encoder
// =============================================================================
void handleEncoder() {
  unsigned char result = encoder.process();

  if (result == DIR_CW) {
    if (frequency + step_size <= FREQ_MAX) {
      frequency += step_size;
      updateVFO();
      display_update_needed = true;
    }
  } else if (result == DIR_CCW) {
    if (frequency >= step_size + FREQ_MIN) {
      frequency -= step_size;
      updateVFO();
      display_update_needed = true;
    }
  }
}

// =============================================================================
// Handle PTT
// =============================================================================
void handlePTT() {
  static bool last_ptt_state = HIGH;
  bool ptt_state = digitalRead(PIN_PTT);

  if (ptt_state != last_ptt_state) {
    if (millis() - last_ptt_time > debounce_delay) {
      last_ptt_time = millis();

      bool new_tx_mode = (ptt_state == LOW);

      if (new_tx_mode != tx_mode) {
        tx_mode = new_tx_mode;
        digitalWrite(PIN_TX_EN, tx_mode ? HIGH : LOW);
        display_update_needed = true;

        Serial.println(tx_mode ? F("TX") : F("RX"));
      }
    }
    last_ptt_state = ptt_state;
  }
}

// =============================================================================
// Handle Encoder Switch (Step Change)
// =============================================================================
void handleEncoderSwitch() {
  static bool last_sw_state = HIGH;
  bool sw_state = digitalRead(PIN_ENC_SW);

  if (sw_state == LOW && last_sw_state == HIGH) {
    if (millis() - last_enc_sw_time > debounce_delay) {
      last_enc_sw_time = millis();

      // Cycle through step sizes
      step_index = (step_index + 1) % num_steps;
      step_size = step_sizes[step_index];
      display_update_needed = true;

      // Save to EEPROM
      saveSettings();

      Serial.print(F("Step: "));
      Serial.println((uint32_t)step_size);
    }
  }
  last_sw_state = sw_state;
}

// =============================================================================
// Update Display
// =============================================================================
void updateDisplay() {
  display.clearDisplay();

  drawFrequency();
  drawStepIndicator();
  drawStatus();

  display.display();
}

// =============================================================================
// Draw Frequency
// =============================================================================
void drawFrequency() {
  uint32_t freq_khz = frequency / 1000;
  uint32_t freq_hz = frequency % 1000;

  // Main frequency display
  display.setTextSize(2);
  display.setCursor(0, 8);

  // Format: 7.XXX.XX MHz
  display.print(freq_khz / 1000);
  display.print(F("."));

  uint16_t khz_part = freq_khz % 1000;
  if (khz_part < 100) display.print(F("0"));
  if (khz_part < 10) display.print(F("0"));
  display.print(khz_part);

  display.print(F("."));

  uint16_t hz_part = freq_hz / 10;
  if (hz_part < 10) display.print(F("0"));
  display.print(hz_part);

  // Unit
  display.setTextSize(1);
  display.setCursor(108, 16);
  display.print(F("MHz"));
}

// =============================================================================
// Draw Step Indicator
// =============================================================================
void drawStepIndicator() {
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print(F("STEP: "));

  switch (step_size) {
    case STEP_10HZ:
      display.print(F("10 Hz"));
      break;
    case STEP_100HZ:
      display.print(F("100 Hz"));
      break;
    case STEP_1KHZ:
      display.print(F("1 kHz"));
      break;
    case STEP_10KHZ:
      display.print(F("10 kHz"));
      break;
  }
}

// =============================================================================
// Draw Status
// =============================================================================
void drawStatus() {
  // TX/RX indicator
  display.setTextSize(2);
  display.setCursor(0, 48);

  if (tx_mode) {
    display.fillRect(0, 46, 32, 18, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.print(F("TX"));
    display.setTextColor(SSD1306_WHITE);
  } else {
    display.drawRect(0, 46, 32, 18, SSD1306_WHITE);
    display.print(F("RX"));
  }

  // Mode indicator
  display.setTextSize(1);
  display.setCursor(45, 52);
  display.print(F("LSB"));

  // Band indicator
  display.setCursor(70, 52);
  display.print(F("40m"));

  // VFO indicator
  display.setCursor(100, 52);
  display.print(F("VFO"));
}

// =============================================================================
// Save Settings to EEPROM
// =============================================================================
void saveSettings() {
  EEPROM.put(EEPROM_FREQ_ADDR, (uint32_t)frequency);
  EEPROM.put(EEPROM_STEP_ADDR, step_index);
  EEPROM.put(EEPROM_MAGIC_ADDR, (uint16_t)EEPROM_MAGIC_VALUE);
}

// =============================================================================
// Load Settings from EEPROM
// =============================================================================
void loadSettings() {
  uint16_t magic;
  EEPROM.get(EEPROM_MAGIC_ADDR, magic);

  if (magic == EEPROM_MAGIC_VALUE) {
    uint32_t saved_freq;
    EEPROM.get(EEPROM_FREQ_ADDR, saved_freq);
    EEPROM.get(EEPROM_STEP_ADDR, step_index);

    // Validate
    if (saved_freq >= FREQ_MIN && saved_freq <= FREQ_MAX) {
      frequency = saved_freq;
    }
    if (step_index >= num_steps) {
      step_index = 2;
    }
    step_size = step_sizes[step_index];

    Serial.println(F("Settings loaded from EEPROM"));
  } else {
    Serial.println(F("Using default settings"));
  }
}
