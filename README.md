# 🎵 ESP32-S3 Zero Digital Audio Player (DAP)

A compact **Digital Audio Player (DAP)** built around the **ESP32-S3 Zero**, capable of playing WAV audio files directly from a microSD card through an I2S DAC/amplifier. The player features a custom OLED-based user interface, hardware navigation buttons, volume control, equalizer adjustment, and persistent user settings.

Designed as a lightweight standalone music player, this project demonstrates how the ESP32-S3 can be used for embedded audio applications with an intuitive user experience.

---

## ✨ Features

- 🎶 WAV audio playback from microSD card
- 📂 Automatic song detection from `/music` directory
- 📺 128×64 SH1106 OLED graphical interface
- 🎮 Four-button navigation system
- ⏯️ Play / Pause functionality
- 🔊 Adjustable volume with EEPROM memory
- 🎚️ 9-Band Equalizer interface
- 🔀 Shuffle mode
- 💾 Persistent settings stored in EEPROM
- 📃 Automatic song list generation
- 🎵 Clean filename display
- ⚡ Smooth real-time audio streaming using I2S

---

## 🛠 Hardware Used

- ESP32-S3 Zero
- SH1106 128×64 OLED Display (SPI)
- MicroSD Card Module
- I2S DAC / Audio Amplifier (MAX98357A or compatible)
- Speaker or Headphones
- 4 Push Buttons
- MicroSD Card
  
---

## 🎵 Supported Audio Format

Current implementation supports:

- WAV (.wav)

Songs should be placed inside:

```
/music
```

Example:

```
SD Card
│
└── music
      ├── Song1.wav
      ├── Song2.wav
      ├── Song3.wav
      └── ...
```

The player automatically scans the folder and loads all available songs during startup.

---

## 🖥 User Interface

The OLED interface includes multiple screens:

### Main Menu

- Songs List
- Volume
- Equalizer
- Settings

### Song List

- Browse available songs
- Wrapped long filenames
- Current playing indicator

### Volume

- Adjustable from 0–100%
- Saved automatically in EEPROM

### Equalizer

9 adjustable frequency bands:

- 62 Hz
- 125 Hz
- 250 Hz
- 500 Hz
- 1 kHz
- 2 kHz
- 4 kHz
- 8 kHz
- 16 kHz

### Settings

- Shuffle ON/OFF
- Stored permanently in EEPROM

---

## 🎮 Controls

| Button | Function |
|---------|----------|
| UP | Navigate Up / Increase Volume / Increase EQ |
| DOWN | Navigate Down / Decrease Volume / Decrease EQ |
| RIGHT | Select / Play / Pause |
| LEFT | Back |

---

## 🔊 Audio Output

Audio is streamed through the ESP32-S3's built-in I2S peripheral.

Supported output devices include:

- MAX98357A
- PCM5102A
- Other compatible I2S DACs
- External I2S Amplifiers

---

## 💾 EEPROM Storage

The player remembers:

- Last volume level
- Shuffle mode

Settings are automatically restored during startup.

---

## 📌 Pin Configuration

### OLED (SPI)

| ESP32-S3 | OLED |
|----------|------|
| GPIO14 | SCK |
| GPIO13 | MOSI |
| GPIO15 | CS |
| GPIO27 | DC |
| GPIO33 | RESET |

---

### SD Card

| ESP32-S3 | SD Module |
|----------|-----------|
| GPIO5 | CS |
| SPI | SCK / MOSI / MISO |

---

### Navigation Buttons

| Button | GPIO |
|---------|------|
| UP | GPIO16 |
| DOWN | GPIO32 |
| LEFT | GPIO21 |
| RIGHT | GPIO17 |

---

### I2S Audio

| ESP32-S3 | I2S |
|----------|-----|
| GPIO26 | BCLK |
| GPIO25 | LRCLK |
| GPIO22 | DOUT |

---

## 📚 Libraries Used

Install the following libraries through the Arduino Library Manager:

- Adafruit GFX
- Adafruit SH110X
- SD
- SPI
- EEPROM
- ESP8266Audio

---

## ⚙ How It Works

1. Initializes the OLED display, SD card, EEPROM, and I2S audio output.
2. Scans the `/music` directory for WAV files.
3. Creates a dynamic playlist.
4. Displays a graphical menu on the OLED.
5. Allows users to browse songs using navigation buttons.
6. Streams audio directly from the SD card over I2S.
7. Stores user settings like volume and shuffle mode.
8. Automatically plays the next song when the current one finishes.

---

## 🚀 Future Improvements

- MP3 support
- FLAC support
- AAC support
- Album artwork display
- Bluetooth audio output
- USB-C file transfer
- Battery level indicator
- Sleep timer
- Playlist support
- Repeat modes
- Real DSP equalizer
- FFT Spectrum Analyzer
- Lyrics display
- RTC-based clock
- Battery-powered portable enclosure

---

## 📸 Project Images

You can add:

- Device front view
- PCB
- Internal wiring
- OLED menu
- Music playback screen
- Completed enclosure

---

## 🤝 Contributions

Contributions, bug reports, and feature suggestions are always welcome.

Feel free to fork the repository and submit a pull request.

---

## 📜 License

This project is released under the MIT License.

---

## 👨‍💻 Author

Designed and developed as a custom embedded **Digital Audio Player (DAP)** using the **ESP32-S3 Zero**, combining embedded systems, digital audio processing, and user interface design into a compact portable music player.
