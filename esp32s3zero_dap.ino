#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <EEPROM.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// --- PINS ---
#define OLED_SCK 14
#define OLED_MOSI 13
#define OLED_CS 15
#define OLED_DC 27
#define OLED_RST 33
#define SD_CS 5
#define BTN_UP    16 
#define BTN_DOWN  32
#define BTN_LEFT  21
#define BTN_RIGHT 17 
#define I2S_BCLK 26
#define I2S_LRC 25
#define I2S_DOUT 22

// --- PROTOTYPES ---
String getCleanName(int index);
void playSong();
void drawUI();

// --- STATE ---
enum View { VIEW_MENU, VIEW_SONGS, VIEW_VOL, VIEW_EQ, VIEW_SETTINGS }; 
View currentView = VIEW_MENU;
enum PlayerState { STATE_STOPPED, STATE_PLAYING, STATE_PAUSED };
PlayerState audioState = STATE_STOPPED;

// --- EQ DATA ---
float eqValues[] = {4.0, 3.5, 4.5, 3.7, 4.2, 5.3, 5.7, 5.0, 3.0}; 
const char* eqFreqs[] = {"62", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"};
int eqCursor = 0; 

// --- GLOBALS ---
String songs[50]; 
int totalSongs = 0, cursor = 0, playingIdx = -1;
float volume = 0.4;
unsigned long lastDisplayUpdate = 0;
bool shuffleMode = false, refreshNeeded = true;

// Separate SPI bus for the OLED (HSPI) to avoid conflict with SD card (VSPI)
SPIClass oledSPI(HSPI);
Adafruit_SH1106G display(128, 64, &oledSPI, OLED_DC, OLED_RST, OLED_CS);

AudioGeneratorWAV *wav = nullptr;
AudioFileSourceSD *file = nullptr;
AudioOutputI2S *out = nullptr;

// --- BUTTON CLASS ---
struct Button {
    uint8_t pin; bool stable, last, pressed; unsigned long lastChange;
    Button(uint8_t p) : pin(p), stable(HIGH), last(HIGH), lastChange(0), pressed(false) {}
    void begin() { pinMode(pin, INPUT_PULLUP); stable = last = digitalRead(pin); }
    void update() {
        bool reading = digitalRead(pin);
        if (reading != last) { last = reading; lastChange = millis(); }
        if ((millis() - lastChange) > 40) { 
            if (reading != stable) { stable = reading; if (stable == LOW) pressed = true; }
        }
    }
    bool wasPressed() { if (pressed) { pressed = false; return true; } return false; }
};
Button btnUp(BTN_UP), btnDown(BTN_DOWN), btnLeft(BTN_LEFT), btnRight(BTN_RIGHT);

// --- HELPER: smart word-wrap drawing ---
int drawWrappedName(String name, int x, int y, bool isSelected, int maxCharsPerLine) {
    if (name.length() == 0) return 0;

    if (isSelected) {
        display.setCursor(0, y);
        display.print(">");
    }

    String line1, line2;
    int len = name.length();
    if (len <= maxCharsPerLine) {
        display.setCursor(x, y);
        display.print(name);
        return 8;
    }

    int breakPos = -1;
    for (int i = maxCharsPerLine; i >= 0; i--) {
        if (name.charAt(i) == ' ') {
            breakPos = i;
            break;
        }
    }
    if (breakPos <= 0) breakPos = maxCharsPerLine;

    line1 = name.substring(0, breakPos);
    line2 = name.substring(breakPos + 1);
    line2.trim();
    if (line2.length() > maxCharsPerLine) {
        line2 = line2.substring(0, maxCharsPerLine - 2) + "..";
    }

    display.setCursor(x, y);
    display.print(line1);
    display.setCursor(x, y + 8);
    display.print(line2);

    return 16;
}

// --- HELPER FUNCTIONS ---
String getCleanName(int index) {
    if (index < 0 || index >= totalSongs) return "";
    String name = songs[index];
    name = name.substring(name.lastIndexOf('/') + 1);
    name.replace(".wav", ""); name.replace(".WAV", "");
    return name;
}

void playSong() {
    if (wav) { wav->stop(); delete wav; wav = nullptr; }
    if (file) { delete file; file = nullptr; }
    file = new AudioFileSourceSD(songs[cursor].c_str());
    wav = new AudioGeneratorWAV();
    wav->begin(file, out);
    playingIdx = cursor;
    audioState = STATE_PLAYING;
    out->SetGain(volume);
}

void drawUI() {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(audioState == STATE_PLAYING ? "PLAYING" : (audioState == STATE_PAUSED ? "PAUSED" : "STOPPED"));
    display.setCursor(100, 0); display.print((int)(volume * 100)); display.print("%");
    display.drawFastHLine(0, 10, 128, SH110X_WHITE);

    if (currentView == VIEW_MENU) {
        const char* menu[] = {"Songs List", "Volume Set", "EQ Settings", "Settings"};
        for(int i=0; i<4; i++) {
            display.setCursor(15, 18 + (i * 11));
            display.print(cursor == i ? "> " : "  "); display.print(menu[i]);
        }
    } 
    else if (currentView == VIEW_SONGS) {
        const int SONGS_PER_PAGE = 2;
        const int MAX_CHARS = 19;
        const int ENTRY_HEIGHT = 20;

        int startIdx = (cursor / SONGS_PER_PAGE) * SONGS_PER_PAGE;
        int yPos = 12;

        for (int i = 0; i < SONGS_PER_PAGE; i++) {
            int idx = startIdx + i;
            if (idx >= totalSongs) break;
            if (yPos > 56) break;

            String name = getCleanName(idx);
            bool isSelected = (idx == cursor);
            drawWrappedName(name, 10, yPos, isSelected, MAX_CHARS);
            yPos += ENTRY_HEIGHT;
        }
    }
    else if (currentView == VIEW_EQ) {
        for (int i = 0; i < 9; i++) {
            int x = 5 + (i * 13);
            int dotY = map(eqValues[i] * 10, -75, 75, 50, 15);
            display.drawFastVLine(x + 5, 15, 35, SH110X_WHITE);
            if (eqCursor == i) {
                display.fillCircle(x + 5, dotY, 3, SH110X_WHITE);
                display.setCursor(x, 55); display.print(eqFreqs[i]);
            } else { display.drawCircle(x + 5, dotY, 2, SH110X_WHITE); }
        }
    }
    else if (currentView == VIEW_VOL) {
        display.setCursor(30, 20); display.print("- VOLUME -");
        display.drawRect(12, 35, 104, 12, 1);
        display.fillRect(14, 37, (int)(volume * 100), 8, 1);
    }
    else if (currentView == VIEW_SETTINGS) {
        display.setCursor(15, 20); display.print(cursor == 0 ? "> Shuffle: " : "  Shuffle: ");
        display.print(shuffleMode ? "ON" : "OFF");
        display.setCursor(15, 32); display.print(cursor == 1 ? "> Back" : "  Back");
    }
    display.display();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- ESP32 Music Player Booting ---");

    if (!EEPROM.begin(10)) {
        Serial.println("EEPROM failed to initialize");
    }
    volume = (EEPROM.read(0) > 100) ? 0.4 : (float)EEPROM.read(0) / 100.0;
    shuffleMode = (EEPROM.read(1) == 1);
    Serial.print("Volume: "); Serial.println(volume);

    btnUp.begin(); btnDown.begin(); btnLeft.begin(); btnRight.begin();
    
    oledSPI.begin(OLED_SCK, -1, OLED_MOSI, OLED_CS);
    if (!display.begin(0, true)) {
        Serial.println("SH1106 allocation failed");
    }
    display.clearDisplay();
    display.display();

    if (SD.begin(SD_CS)) {
        Serial.println("SD Card Mounted Successfully.");
        
        File root = SD.open("/music");
        if (!root) {
            Serial.println("ERROR: /music folder not found! Create a folder named 'music' on the SD.");
        } else {
            while (File entry = root.openNextFile()) {
                if (!entry.isDirectory()) {
                    String n = String(entry.name());
                    String nLower = n;
                    nLower.toLowerCase();
                    if (nLower.endsWith(".wav") && totalSongs < 50) {
                        if (n.startsWith("/music/")) {
                            songs[totalSongs++] = n;
                        } else {
                            songs[totalSongs++] = "/music/" + n;
                        }
                        Serial.print("Loaded: "); Serial.println(songs[totalSongs-1]);
                    }
                }
                entry.close();
            }
            root.close();
            Serial.print("Total Songs Found: "); Serial.println(totalSongs);
        }
    } else {
        Serial.println("SD Card Mount FAILED! Check wiring (CS=5, MOSI=23, MISO=19, SCK=18)");
    }

    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetGain(volume);
    
    randomSeed(analogRead(34));
    Serial.println("Setup Complete.");
}

void loop() {
    // Tight audio feed loop
    if (audioState == STATE_PLAYING && wav && wav->isRunning()) {
        for (int i = 0; i < 8 && wav->isRunning(); i++) {
            if (!wav->loop()) break;
        }
        if (!wav->isRunning()) {
            cursor = shuffleMode ? random(0, totalSongs) : (playingIdx + 1) % totalSongs;
            playSong();
            refreshNeeded = true;
        }
    } else if (out) {
        int16_t sil[2] = {0,0};
        out->ConsumeSample(sil);
    }

    // Process button inputs
    btnUp.update(); btnDown.update(); btnLeft.update(); btnRight.update();

    if (btnLeft.wasPressed()) {
        if (currentView == VIEW_EQ && eqCursor > 0) eqCursor--;
        else if (currentView == VIEW_SONGS) {
            // Remember where we were so we can return to the active track later
            if (audioState != STATE_STOPPED) playingIdx = cursor;
            currentView = VIEW_MENU; 
            cursor = 0;
        }
        else { currentView = VIEW_MENU; cursor = 0; }
        refreshNeeded = true;
    }
    if (btnRight.wasPressed()) {
        if (currentView == VIEW_MENU) {
            if (cursor == 0) {
                currentView = VIEW_SONGS;
                // Jump to the currently playing/paused song, otherwise start at top
                if (audioState != STATE_STOPPED && playingIdx >= 0 && playingIdx < totalSongs) {
                    cursor = playingIdx;
                } else {
                    cursor = 0;
                }
                // no cursor=0 here – we keep the value above
            }
            else if (cursor == 1) {
                currentView = VIEW_VOL;
                cursor = 0;      // reset cursor for volume view
            }
            else if (cursor == 2) {
                currentView = VIEW_EQ;
                cursor = 0;
                eqCursor = 0;
            }
            else if (cursor == 3) {
                currentView = VIEW_SETTINGS;
                cursor = 0;
            }
        } 
        else if (currentView == VIEW_SONGS) {
            if (playingIdx == cursor && audioState != STATE_STOPPED) 
                audioState = (audioState == STATE_PLAYING) ? STATE_PAUSED : STATE_PLAYING;
            else playSong();
        }
        else if (currentView == VIEW_EQ && eqCursor < 8) eqCursor++;
        else if (currentView == VIEW_SETTINGS && cursor == 0) { 
            shuffleMode = !shuffleMode; 
            EEPROM.write(1, shuffleMode); 
            EEPROM.commit(); 
        }
        refreshNeeded = true;
    }
    if (btnUp.wasPressed()) {
        if (currentView == VIEW_EQ) eqValues[eqCursor] = min(7.5f, eqValues[eqCursor] + 0.5f);
        else if (currentView == VIEW_VOL) { 
            volume = min(1.0f, volume + 0.05f); 
            out->SetGain(volume); 
            EEPROM.write(0, (int)(volume*100)); 
            EEPROM.commit(); 
        }
        else { 
            int lim = (currentView == VIEW_MENU) ? 4 : (currentView == VIEW_SETTINGS ? 2 : totalSongs); 
            cursor = (cursor - 1 + lim) % lim; 
        }
        refreshNeeded = true;
    }
    if (btnDown.wasPressed()) {
        if (currentView == VIEW_EQ) eqValues[eqCursor] = max(-7.5f, eqValues[eqCursor] - 0.5f);
        else if (currentView == VIEW_VOL) { 
            volume = max(0.0f, volume - 0.05f); 
            out->SetGain(volume); 
            EEPROM.write(0, (int)(volume*100)); 
            EEPROM.commit(); 
        }
        else { 
            int lim = (currentView == VIEW_MENU) ? 4 : (currentView == VIEW_SETTINGS ? 2 : totalSongs); 
            cursor = (cursor + 1) % lim; 
        }
        refreshNeeded = true;
    }

    if (refreshNeeded || audioState == STATE_PLAYING) {
        if (millis() - lastDisplayUpdate > 100) { 
            drawUI(); 
            lastDisplayUpdate = millis(); 
            refreshNeeded = false; 
        }
    }
}
