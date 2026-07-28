#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ---------------- Display ----------------
Adafruit_SSD1306 display(128, 64, &Wire, -1);
#define OLED_ADDR 0x3C            // <-- set to whatever your I2C scanner reported (0x3C or 0x3D)

// ---------------- DFPlayer ----------------
// SoftwareSerial(RX, TX): D2 = Nano RX  <- DFPlayer TX
//                         D3 = Nano TX  -> DFPlayer RX (through 1k resistor)
SoftwareSerial dfSerial(2, 3);
DFRobotDFPlayerMini player;

// ---------------- Buttons ----------------
const uint8_t BTN_PLAY = 4;
const uint8_t BTN_NEXT = 5;
const uint8_t BTN_PREV = 6;
const uint8_t buttons[] = {BTN_PLAY, BTN_NEXT, BTN_PREV};
const uint8_t NUM_BTNS = 3;

uint8_t lastReading[NUM_BTNS], stableState[NUM_BTNS];
unsigned long lastChange[NUM_BTNS];
const unsigned long DEBOUNCE_MS = 30;

// ---------------- Track table (placeholders, stored in flash) ----------------
const uint8_t NUM_TRACKS = 15;

const char t0[]  PROGMEM = "Track 01";   const char a0[]  PROGMEM = "Artist 01";
const char t1[]  PROGMEM = "Track 02";   const char a1[]  PROGMEM = "Artist 02";
const char t2[]  PROGMEM = "Track 03";   const char a2[]  PROGMEM = "Artist 03";
const char t3[]  PROGMEM = "Track 04";   const char a3[]  PROGMEM = "Artist 04";
const char t4[]  PROGMEM = "Track 05";   const char a4[]  PROGMEM = "Artist 05";
const char t5[]  PROGMEM = "Track 06";   const char a5[]  PROGMEM = "Artist 06";
const char t6[]  PROGMEM = "Track 07";   const char a6[]  PROGMEM = "Artist 07";
const char t7[]  PROGMEM = "Track 08";   const char a7[]  PROGMEM = "Artist 08";
const char t8[]  PROGMEM = "Track 09";   const char a8[]  PROGMEM = "Artist 09";
const char t9[]  PROGMEM = "Track 10";   const char a9[]  PROGMEM = "Artist 10";
const char t10[] PROGMEM = "Track 11";   const char a10[] PROGMEM = "Artist 11";
const char t11[] PROGMEM = "Track 12";   const char a11[] PROGMEM = "Artist 12";
const char t12[] PROGMEM = "Track 13";   const char a12[] PROGMEM = "Artist 13";
const char t13[] PROGMEM = "Track 14";   const char a13[] PROGMEM = "Artist 14";
const char t14[] PROGMEM = "Track 15";   const char a14[] PROGMEM = "Artist 15";

const char* const titles[]  PROGMEM = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14};
const char* const artists[] PROGMEM = {a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14};

// scratch for copying one string out of flash.
// MUST be >= longest title/artist length + 1. Bump this if your real names are longer.
char nameBuf[24];

void getTitle(uint8_t i)  { strncpy_P(nameBuf, (char*)pgm_read_word(&titles[i]),  sizeof(nameBuf)); nameBuf[sizeof(nameBuf)-1] = '\0'; }
void getArtist(uint8_t i) { strncpy_P(nameBuf, (char*)pgm_read_word(&artists[i]), sizeof(nameBuf)); nameBuf[sizeof(nameBuf)-1] = '\0'; }

// ---------------- Player state ----------------
int     volume    = 20;     // 0..30 — fixed at startup (no volume buttons in this build)
uint8_t track     = 0;      // 0-based index; DFPlayer file = track+1
bool    isPlaying = false;

void setup() {
  Serial.begin(115200);
  delay(500);

  for (uint8_t i = 0; i < NUM_BTNS; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    lastReading[i] = HIGH; stableState[i] = HIGH; lastChange[i] = 0;
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED fail")); while (true);
  }

  dfSerial.begin(9600);
  if (!player.begin(dfSerial)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println(F("DFPlayer not found"));
    display.display();
    Serial.println(F("DFPlayer not found - check TX/RX and SD card"));
    while (true);
  }

  player.volume(volume);
  updateDisplay();
}

bool pressed(uint8_t i) {
  uint8_t r = digitalRead(buttons[i]);
  if (r != lastReading[i]) { lastChange[i] = millis(); lastReading[i] = r; }
  if (millis() - lastChange[i] > DEBOUNCE_MS) {
    if (r != stableState[i]) { stableState[i] = r; if (r == LOW) return true; }
  }
  return false;
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Track title (size 2)
  display.setTextSize(2);
  display.setCursor(0, 0);
  getTitle(track);
  display.println(nameBuf);

  // Artist (size 1)
  display.setTextSize(1);
  display.setCursor(0, 20);
  getArtist(track);
  display.println(nameBuf);

  // Track number + play state
  display.setCursor(0, 34);
  display.print(F("#"));
  display.print(track + 1);
  display.print(F("/"));
  display.print(NUM_TRACKS);
  display.print(F("  "));
  display.print(isPlaying ? F(">PLAY") : F("=PAUSE"));

  // Volume readout (fixed value, no bar-editing since there are no vol buttons)
  display.setCursor(0, 48);
  display.print(F("Vol "));
  display.print(volume);
  int barW = map(volume, 0, 30, 0, 90);
  display.drawRect(34, 56, 92, 6, SSD1306_WHITE);
  display.fillRect(35, 57, barW, 4, SSD1306_WHITE);

  display.display();
}

void playCurrent() {
  player.play(track + 1);   // DFPlayer files are 1-based
  isPlaying = true;
  updateDisplay();
}

void loop() {
  if (pressed(0)) {  // PLAY / PAUSE
    if (isPlaying) { player.pause(); isPlaying = false; }
    else           { player.start(); isPlaying = true;  }
    updateDisplay();
  }

  if (pressed(1)) {  // NEXT
    track = (track + 1) % NUM_TRACKS;
    playCurrent();
  }

  if (pressed(2)) {  // PREVIOUS
    track = (track == 0) ? NUM_TRACKS - 1 : track - 1;
    playCurrent();
  }
}
