/*  
 * ____                     _      ______ _____    _____
  / __ \                   | |    |  ____|  __ \  |  __ \               
 | |  | |_ __   ___ _ __   | |    | |__  | |  | | | |__) |__ _  ___ ___ 
 | |  | | '_ \ / _ \ '_ \  | |    |  __| | |  | | |  _  // _` |/ __/ _ \
 | |__| | |_) |  __/ | | | | |____| |____| |__| | | | \ \ (_| | (_|  __/
  \____/| .__/ \___|_| |_| |______|______|_____/  |_|  \_\__,_|\___\___|
        | |                                                             
        |_|          
 Open LED Race
 A minimalist cars race for LED strip  
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Original project by: gbarbarov@singulardevices.com for Arduino Day Seville 2019 
 
 Code made dirty and fast, next improvements at: 
 https://github.com/gbarbarov/led-race
 https://www.hackster.io/gbarbarov/open-led-race-a0331a
 https://twitter.com/openledrace

 ------------------------------------------------------------------------
 NOW LED Race 900 - Major upgrades by Marcello Caselli (mcasel98) Jan 2026
 Full rewrite, race logic, and advanced features powered by GitHub Copilot
 Special thanks to OpenAI Copilot for assistance, code review and many race-inspired ideas!

 Key improvements:
 - Welcome message
 - Box people animation before race
 - Count down: Ready-Set-Go!
 - Race leader monitoring
 - Laps counter, Last lap alert, Winner declaration
 - Podium light show
 - Winner music
 - Return racing cars to the starting line before next race
 - Fully non-blocking game logic: "FINAL LAP!!" always shows completely, racing never pauses

 Released under the same GPLv3 license.
 For full release and source: https://github.com/mcasel98/Led-Race900
*/

#include <Adafruit_NeoPixel.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <avr/wdt.h>  // For watchdog timer reset

constexpr int NUM_CARS     = 5;
constexpr int NUM_LAPS     = 4;
constexpr int TRACK_PIXELS = 900;

constexpr int PIN_LED_DATA       = A0;
constexpr int PIN_AUDIO          = 3;
constexpr int PIN_START          = 8;
constexpr int PIN_CAR[NUM_CARS]  = {7, 6, 5, 4, 2};

constexpr int LED_MATRIX_CS      = 10;
constexpr int LED_MATRIX_DIN     = 9;
constexpr int LED_MATRIX_CLK     = 11;
constexpr int LED_MATRIX_DEV     = 4;
constexpr auto LED_MATRIX_HW     = MD_MAX72XX::FC16_HW;

//constexpr float ACCELERATION     = 0.126f;
constexpr float ACCELERATION     = 0.150f;
constexpr float FRICTION         = 0.012f;
constexpr float GRAVITY_CONST    = 0.003f;
constexpr int   GRAVITY_BASE     = 127;

constexpr int ANIM_FRAME_MS      = 25;
constexpr int COUNTDOWN_STEP_MS  = 1000;

constexpr const char* CAR_COLOR_NAMES[NUM_CARS] = {
  "RED", "GREEN", "BLUE", "WHITE", "YELLOW"
};

enum class RaceState : uint8_t { WAITING, RACING, WINNER, RESULTS, RESET_CARS, DEMO_RACING, DEMO_WINNER };

struct Car {
  float distance    = 0;
  float speed       = 0;
  uint8_t laps      = 0;
  bool accel_ready  = true;
};

Car cars[NUM_CARS];
uint8_t gravity_map[TRACK_PIXELS];
uint32_t CAR_COLORS[NUM_CARS];

MD_Parola matrix(LED_MATRIX_HW, LED_MATRIX_DIN, LED_MATRIX_CLK, LED_MATRIX_CS, LED_MATRIX_DEV);
Adafruit_NeoPixel track(TRACK_PIXELS, PIN_LED_DATA, NEO_GRB + NEO_KHZ800);

RaceState gameState           = RaceState::WAITING;
char win_msg[80]             = {'\0'};

int tbeep                    = 0;
bool welcomeMessageSet       = false;
int winnerIdx                = -1;
bool winner_announced        = false;

constexpr int WIN_MUSIC[] = {2637, 2637, 0, 2637, 0, 2093, 2637, 0, 3136};

unsigned long winnerShowMillis = 0;
int finishingOrder[NUM_CARS] = {0};
unsigned long lastBlinkMs = 0;
bool resultBlinkState = false;
int resetCarIndex = 0;
bool resetCarAnimating = false;
float lerpPos = 0.0;

// --- State management for FINAL LAP message ---
bool showFinalLapActive = false;
bool finalLapShownOnce = false;
unsigned long finalLapLedStart = 0;
bool finalLapLedEffectDone = false;

/*** MATRIX STATE ***/
bool showingLapMsg = false;
int currentLeaderIdx = -1;
char lastWinnerText[80] = "";

/*** DEBOUNCE START BUTTON ***/
constexpr unsigned long DEBOUNCE_MS = 50;
bool startBtnState = HIGH;
bool lastStartBtnReading = HIGH;
unsigned long lastDebounceTime = 0;

/*** DEMO MODE - COMPLETELY ISOLATED ***/
// DEMO state variables with DEMO_ prefix for complete isolation
bool DEMO_active = false;
Car DEMO_cars[NUM_CARS];
int DEMO_tbeep = 0;
int DEMO_winnerIdx = -1;
bool DEMO_winner_announced = false;
unsigned long DEMO_winnerShowMillis = 0;
bool DEMO_showFinalLapActive = false;
bool DEMO_finalLapShownOnce = false;
unsigned long DEMO_finalLapLedStart = 0;
bool DEMO_finalLapLedEffectDone = false;
char DEMO_lastWinnerText[80] = "";

// Virtual player simulation variables
struct DEMO_VirtualPlayer {
  float pressFrequency;        // Current press frequency in Hz (2-5 Hz)
  unsigned long lastPressTime;
  unsigned long nextFreqChange;
  unsigned long lastFreqChangeTime;
};
DEMO_VirtualPlayer DEMO_players[NUM_CARS];

// START button hold detection for mode entry/exit
unsigned long startPressedTime = 0;
bool startWasPressed = false;

/*** BASE GRAPHICS ***/
void drawBaseTrack() {
  for (int i = 0; i < TRACK_PIXELS; ++i)
    track.setPixelColor(i, track.Color(0, 0, (GRAVITY_BASE - gravity_map[i]) / 12));
}

void drawBoxAnimation() {
  for (int i = 0; i < (TRACK_PIXELS > 20 ? 20 : TRACK_PIXELS); ++i) {
    int b = random(0, 13);
    if (b > 10)
      track.setPixelColor(i, track.Color(random(10, 40), random(10, 40), random(10, 40)));
    else
      track.setPixelColor(i, 0);
  }
  track.show();
  delay(5);
}

void drawCars() {
  int order[NUM_CARS];
  for (int i = 0; i < NUM_CARS; ++i) order[i] = i;
  for (int i = 0; i < NUM_CARS - 1; ++i)
    for (int j = 0; j < NUM_CARS - 1 - i; ++j)
      if (cars[order[j]].distance > cars[order[j + 1]].distance) {
        int temp = order[j]; order[j] = order[j + 1]; order[j + 1] = temp;
      }
  for (int i = 0; i < NUM_CARS; ++i) {
    int idx = order[i];
    int pos = static_cast<int>(cars[idx].distance) % TRACK_PIXELS;
    track.setPixelColor(pos, CAR_COLORS[idx]);
    int tailpos = (pos > 0) ? pos - 1 : TRACK_PIXELS - 1;
    track.setPixelColor(tailpos, CAR_COLORS[idx] & 0x2F2F2F);
  }
}

void resetRace() {
  memset(cars, 0, sizeof(cars));
  tbeep = 0;
  winnerIdx = -1;
  winnerShowMillis = 0;
  lastBlinkMs = 0;
  showingLapMsg = false;
  winner_announced = false;
  currentLeaderIdx = -1;
  showFinalLapActive = false;
  finalLapShownOnce = false;
  finalLapLedStart = 0;
  finalLapLedEffectDone = false;
  lastWinnerText[0]=0;
}

/*** SAFE ARDUINO RESET FUNCTION ***/
void performSafeReset() {
  // Disable interrupts
  cli();
  // Enable watchdog with shortest timeout
  wdt_enable(WDTO_15MS);
  // Wait for watchdog reset
  while(1) {}
}

/*** DEBOUNCE START BUTTON ***/
bool debounceStartButton() {
  bool reading = digitalRead(PIN_START);
  
  if (reading != lastStartBtnReading) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (reading != startBtnState) {
      startBtnState = reading;
    }
  }
  
  lastStartBtnReading = reading;
  return (startBtnState == LOW);
}

/*** DEMO MODE FUNCTIONS - COMPLETELY ISOLATED ***/
void DEMO_resetRace() {
  memset(DEMO_cars, 0, sizeof(DEMO_cars));
  DEMO_tbeep = 0;
  DEMO_winnerIdx = -1;
  DEMO_winnerShowMillis = 0;
  DEMO_winner_announced = false;
  DEMO_showFinalLapActive = false;
  DEMO_finalLapShownOnce = false;
  DEMO_finalLapLedStart = 0;
  DEMO_finalLapLedEffectDone = false;
  DEMO_lastWinnerText[0] = 0;
  
  // Initialize virtual players
  for (int i = 0; i < NUM_CARS; ++i) {
    DEMO_players[i].pressFrequency = random(200, 501) / 100.0f; // 2-5 Hz
    DEMO_players[i].lastPressTime = 0;
    DEMO_players[i].nextFreqChange = random(1000, 3501); // 1-3.5 seconds
    DEMO_players[i].lastFreqChangeTime = millis();
  }
}

void DEMO_updateVirtualPlayers() {
  unsigned long now = millis();
  
  for (int i = 0; i < NUM_CARS; ++i) {
    // Update press frequency periodically (every 1-3.5 seconds)
    if (now - DEMO_players[i].lastFreqChangeTime >= DEMO_players[i].nextFreqChange) {
      DEMO_players[i].pressFrequency = random(200, 501) / 100.0f; // 2-5 Hz
      DEMO_players[i].nextFreqChange = random(1000, 3501); // Next change in 1-3.5 seconds
      DEMO_players[i].lastFreqChangeTime = now;
    }
    
    // Simulate button press based on frequency
    unsigned long pressPeriod = (unsigned long)(1000.0f / DEMO_players[i].pressFrequency);
    
    if (now - DEMO_players[i].lastPressTime >= pressPeriod) {
      if (DEMO_cars[i].accel_ready) {
        DEMO_cars[i].speed += ACCELERATION;
        DEMO_cars[i].accel_ready = false;
      }
      DEMO_players[i].lastPressTime = now;
    } else {
      DEMO_cars[i].accel_ready = true;
    }
  }
}

void DEMO_updatePhysics() {
  for (int i = 0; i < NUM_CARS; ++i) {
    int pos = static_cast<int>(DEMO_cars[i].distance) % TRACK_PIXELS;
    if (gravity_map[pos] < GRAVITY_BASE)
      DEMO_cars[i].speed -= GRAVITY_CONST * (GRAVITY_BASE - gravity_map[pos]);
    else if (gravity_map[pos] > GRAVITY_BASE)
      DEMO_cars[i].speed += GRAVITY_CONST * (gravity_map[pos] - GRAVITY_BASE);

    DEMO_cars[i].speed -= DEMO_cars[i].speed * FRICTION;
    DEMO_cars[i].distance += DEMO_cars[i].speed;
    if (DEMO_cars[i].distance > TRACK_PIXELS * (DEMO_cars[i].laps + 1)) {
      ++DEMO_cars[i].laps;
      tone(PIN_AUDIO, 700 + 100 * i);
      DEMO_tbeep = 3;
    }
  }
}

void DEMO_drawCars() {
  int order[NUM_CARS];
  for (int i = 0; i < NUM_CARS; ++i) order[i] = i;
  for (int i = 0; i < NUM_CARS - 1; ++i)
    for (int j = 0; j < NUM_CARS - 1 - i; ++j)
      if (DEMO_cars[order[j]].distance > DEMO_cars[order[j + 1]].distance) {
        int temp = order[j]; order[j] = order[j + 1]; order[j + 1] = temp;
      }
  for (int i = 0; i < NUM_CARS; ++i) {
    int idx = order[i];
    int pos = static_cast<int>(DEMO_cars[idx].distance) % TRACK_PIXELS;
    track.setPixelColor(pos, CAR_COLORS[idx]);
    int tailpos = (pos > 0) ? pos - 1 : TRACK_PIXELS - 1;
    track.setPixelColor(tailpos, CAR_COLORS[idx] & 0x2F2F2F);
  }
}

void DEMO_displayRaceMessage() {
  static char currentDisplayBuf[64] = "";
  static char lastTextSet[64] = "";
  static int lastLapShowed = -2;
  int leader_idx = 0, lapNow = 0; float leaderDist = -1;

  for (int i = 0; i < NUM_CARS; ++i) {
    if (DEMO_cars[i].laps > lapNow || (DEMO_cars[i].laps == lapNow && DEMO_cars[i].distance > leaderDist)) {
      leader_idx = i; lapNow = DEMO_cars[i].laps; leaderDist = DEMO_cars[i].distance;
    }
  }

  // Handle "FINAL LAP!!"
  if (!DEMO_showFinalLapActive && !DEMO_finalLapShownOnce && lapNow == NUM_LAPS-1) {
    DEMO_showFinalLapActive = true;
    DEMO_finalLapLedStart = millis();
    DEMO_finalLapLedEffectDone = false;
    matrix.displayClear();
    matrix.displayText("   FINAL LAP!!   ", PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(lastTextSet, "FINAL LAP");
  }
  if (DEMO_showFinalLapActive) {
    if (!DEMO_finalLapLedEffectDone) {
      unsigned long animTime = millis() - DEMO_finalLapLedStart;
      int animStep = animTime / 400;
      uint32_t c = (animStep==1) ? track.Color(180,0,180) : track.Color(220,220,255);
      for (int i=0; i<30 && i<TRACK_PIXELS; ++i) track.setPixelColor(i, c);
      track.show();
      if (animStep >= 2) {
        for (int i=0; i<30 && i<TRACK_PIXELS; ++i) track.setPixelColor(i,0);
        track.show();
        DEMO_finalLapLedEffectDone = true;
      }
    }
    if (matrix.displayAnimate()) {
      matrix.displayReset();
      DEMO_showFinalLapActive = false;
      DEMO_finalLapShownOnce = true;
      lastTextSet[0]=0;
    }
    return;
  }

  // Lap n/4
  if (lapNow != lastLapShowed) {
    snprintf(currentDisplayBuf, sizeof(currentDisplayBuf), "   %d/%d   ", (lapNow < NUM_LAPS ? lapNow+1 : NUM_LAPS), NUM_LAPS);
    if (strcmp(lastTextSet, currentDisplayBuf) != 0) {
      matrix.displayClear();
      matrix.displayText(currentDisplayBuf, PA_CENTER, 6, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      strcpy(lastTextSet, currentDisplayBuf);
    }
    if (matrix.displayAnimate()) {
      matrix.displayReset();
      lastLapShowed = lapNow;
      lastTextSet[0]=0;
    }
    return;
  }

  snprintf(currentDisplayBuf, sizeof(currentDisplayBuf), "   %s   1st   ", CAR_COLOR_NAMES[leader_idx]);
  if (strcmp(lastTextSet, currentDisplayBuf) != 0) {
    matrix.displayClear();
    matrix.displayText(currentDisplayBuf, PA_CENTER, 6, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(lastTextSet, currentDisplayBuf);
  }
  if (matrix.displayAnimate()) {
    matrix.displayReset();
    matrix.displayText(currentDisplayBuf, PA_CENTER, 6, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(lastTextSet, currentDisplayBuf);
  }
}

void DEMO_displayWinnerMessage() {
  int safeWinnerIdx = DEMO_winnerIdx;
  if (safeWinnerIdx < 0 || safeWinnerIdx >= NUM_CARS) safeWinnerIdx = 0;
  char win_msg_demo[80];
  snprintf(win_msg_demo, sizeof(win_msg_demo), "   %s   WINS   THE   RACE!   [DEMO]   ", CAR_COLOR_NAMES[safeWinnerIdx]);
  if (strcmp(DEMO_lastWinnerText, win_msg_demo) != 0) {
    matrix.displayClear();
    matrix.displayText(win_msg_demo, PA_CENTER, 28, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(DEMO_lastWinnerText, win_msg_demo);
  }
  if (matrix.displayAnimate()) {
    matrix.displayReset();
    matrix.displayText(win_msg_demo, PA_CENTER, 28, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }
}

void DEMO_playWinnerFx(int carIdx) {
  for (int i = 0; i < (TRACK_PIXELS > 30 ? 30 : TRACK_PIXELS); ++i) track.setPixelColor(i, CAR_COLORS[carIdx]);
  track.show();
  int sz = sizeof(WIN_MUSIC) / sizeof(int);
  for (int note = 0; note < sz; ++note) {
    if (WIN_MUSIC[note] > 0) tone(PIN_AUDIO, WIN_MUSIC[note], 200);
    delay(200); noTone(PIN_AUDIO);
  }
  DEMO_winnerIdx = carIdx;
}

void DEMO_checkForWinner() {
  if (DEMO_winner_announced) return;
  for (int i = 0; i < NUM_CARS; ++i) {
    if (DEMO_cars[i].laps >= NUM_LAPS) {
      DEMO_winner_announced = true;
      DEMO_playWinnerFx(i);
      gameState = RaceState::DEMO_WINNER;
      DEMO_winnerShowMillis = millis();
      break;
    }
  }
}

/*** ENGLISH COUNTDOWN: RED-YELLOW-GREEN LIGHTS ***/
void raceCountdown() {
  struct {
    const char* text;
    uint32_t color;
    int freq;
  } steps[] = {
    {"   READY   ",   track.Color(255,0,0), 660},
    {"   SET   ",     track.Color(255,255,0), 880},
    {"   GO!   ",     track.Color(0,255,0),  1320}
  };
  for (int s=0; s<3; ++s) {
    for (int i=0; i<50 && i<TRACK_PIXELS; ++i)
      track.setPixelColor(i, steps[s].color);
    for (int i=50; i<TRACK_PIXELS; ++i)
      track.setPixelColor(i, 0);
    track.show();

    matrix.displayClear();
    matrix.displayText(steps[s].text, PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    while (!matrix.displayAnimate()) {}
    matrix.displayReset();

    tone(PIN_AUDIO, steps[s].freq);
    delay(COUNTDOWN_STEP_MS);
    noTone(PIN_AUDIO);

    for (int i=0; i<50 && i<TRACK_PIXELS; ++i)
      track.setPixelColor(i, track.Color(220,220,220));
    track.show();
    delay(70);
    for (int i=0; i<50 && i<TRACK_PIXELS; ++i)
      track.setPixelColor(i, 0);
    track.show();
    delay(130);
  }
}

/*** ANIMATED RESET TO BOXES ***/
void animateCarReset() {
  static int startPos = 0;
  if (!resetCarAnimating) {
    startPos = static_cast<int>(cars[resetCarIndex].distance) % TRACK_PIXELS;
    lerpPos = 0.0;
    resetCarAnimating = true;
  }
  float step = (startPos) / 20.0;
  lerpPos += step;
  int newPos = startPos - static_cast<int>(lerpPos);
  if (newPos < 0) newPos = 0;
  int tail = 9;
  for (int i=0; i<TRACK_PIXELS; ++i) track.setPixelColor(i, 0);
  for (int t=0; t<=tail; ++t) {
    int tailpos = newPos + t;
    if (tailpos >= startPos) break;
    uint8_t fade = 200/(t+2);
    uint32_t color = CAR_COLORS[resetCarIndex];
    uint8_t r = (uint8_t)((color>>16)&0xFF), g = (uint8_t)((color>>8)&0xFF), b = (uint8_t)((color)&0xFF);
    track.setPixelColor(tailpos, track.Color((r*fade)>>8,(g*fade)>>8,(b*fade)>>8));
  }
  track.setPixelColor(newPos, CAR_COLORS[resetCarIndex]);
  track.show();
  if (lerpPos >= startPos) {
    cars[resetCarIndex].distance = 0;
    lerpPos = 0.0;
    resetCarIndex++;
    resetCarAnimating = false;
    delay(200);
  }
}

/*** RACE MESSAGES DISPLAY - FINAL LAP ALWAYS COMPLETE ***/
void displayRaceEssentialMessage() {
  static char currentDisplayBuf[64] = "";
  static char lastTextSet[64] = "";
  static int lastLapShowed = -2;
  int leader_idx = 0, lapNow = 0; float leaderDist = -1;

  for (int i = 0; i < NUM_CARS; ++i) {
    if (cars[i].laps > lapNow || (cars[i].laps == lapNow && cars[i].distance > leaderDist)) {
      leader_idx = i; lapNow = cars[i].laps; leaderDist = cars[i].distance;
    }
  }

  // Show "FINAL LAP!!" completely, no message interruption, once
  if (!showFinalLapActive && !finalLapShownOnce && lapNow == NUM_LAPS-1) {
    showFinalLapActive = true;
    finalLapLedStart = millis();
    finalLapLedEffectDone = false;
    matrix.displayClear();
    matrix.displayText("   FINAL LAP!!   ", PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(lastTextSet, "FINAL LAP");
  }
  if (showFinalLapActive) {
    // Led animation (non-blocking, quick)
    if (!finalLapLedEffectDone) {
      unsigned long animTime = millis() - finalLapLedStart;
      int animStep = animTime / 400;
      uint32_t c = (animStep==1) ? track.Color(180,0,180) : track.Color(220,220,255);
      for (int i=0; i<30 && i<TRACK_PIXELS; ++i) track.setPixelColor(i, c);
      track.show();
      if (animStep >= 2) {
        for (int i=0; i<30 && i<TRACK_PIXELS; ++i) track.setPixelColor(i,0);
        track.show();
        finalLapLedEffectDone = true;
      }
    }
    if (matrix.displayAnimate()) {
      matrix.displayReset();
      showFinalLapActive = false;
      finalLapShownOnce = true;
      lastTextSet[0]=0;
    }
    return;
  }

  // LAP X/Y short message (fast scroll)
  if (lapNow != lastLapShowed) {
    snprintf(currentDisplayBuf, sizeof(currentDisplayBuf), "   %d/%d   ", (lapNow < NUM_LAPS ? lapNow+1 : NUM_LAPS), NUM_LAPS);
    if (strcmp(lastTextSet, currentDisplayBuf) != 0) {
      matrix.displayClear();
      matrix.displayText(currentDisplayBuf, PA_CENTER, 6, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      strcpy(lastTextSet, currentDisplayBuf);
    }
    if (matrix.displayAnimate()) {
      matrix.displayReset();
      lastLapShowed = lapNow;
      lastTextSet[0]=0;
    }
    return;
  }

  snprintf(currentDisplayBuf, sizeof(currentDisplayBuf), "   %s   1o   ", CAR_COLOR_NAMES[leader_idx]);
  if (strcmp(lastTextSet, currentDisplayBuf) != 0) {
    matrix.displayClear();
    matrix.displayText(currentDisplayBuf, PA_CENTER, 6, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(lastTextSet, currentDisplayBuf);
  }
  if (matrix.displayAnimate()) {
    matrix.displayReset();
    matrix.displayText(currentDisplayBuf, PA_CENTER, 6, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(lastTextSet, currentDisplayBuf);
  }
}

void displayWinnerLoopMessage() {
  int safeWinnerIdx = winnerIdx;
  if (safeWinnerIdx < 0 || safeWinnerIdx >= NUM_CARS) safeWinnerIdx = 0;
  snprintf(win_msg, sizeof(win_msg), "   %s   WINS   THE   RACE!      ", CAR_COLOR_NAMES[safeWinnerIdx]);
  if (strcmp(lastWinnerText, win_msg) != 0) {
    matrix.displayClear();
    matrix.displayText(win_msg, PA_CENTER, 28, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(lastWinnerText, win_msg);
  }
  if (matrix.displayAnimate()) {
    matrix.displayReset();
    matrix.displayText(win_msg, PA_CENTER, 28, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }
}

void computeFinishingOrder() {
  struct CarRank { int idx; float laps; float dist; } cr[NUM_CARS];
  for (int i = 0; i < NUM_CARS; ++i) {
    cr[i].idx = i; cr[i].laps = cars[i].laps; cr[i].dist = cars[i].distance;
  }
  for (int i = 0; i < NUM_CARS - 1; ++i)
    for (int j = 0; j < NUM_CARS - 1 - i; ++j)
      if (cr[j].laps < cr[j + 1].laps ||
        (cr[j].laps == cr[j + 1].laps && cr[j].dist < cr[j + 1].dist)) {
        CarRank temp = cr[j]; cr[j] = cr[j + 1]; cr[j + 1] = temp;
      }
  for (int i = 0; i < NUM_CARS; ++i) finishingOrder[i] = cr[i].idx;
}

void drawRaceResultBox() {
  if (millis() - lastBlinkMs > 1000) {
    resultBlinkState = !resultBlinkState;
    lastBlinkMs = millis();
  }
  for (int i = 0; i < TRACK_PIXELS; ++i) track.setPixelColor(i, 0);
  int boxGap = 4;
  for (int p = 0; p < NUM_CARS && (p + 1) * boxGap <= 20; ++p) {
    int idx = finishingOrder[NUM_CARS - 1 - p];
    uint32_t color = CAR_COLORS[idx];
    if (p == NUM_CARS - 1 && resultBlinkState) color = 0;
    for (int k = 0; k < boxGap - 1; ++k)
      track.setPixelColor(p * boxGap + k, color);
    track.setPixelColor(p * boxGap + (boxGap - 1), 0x030303);
  }
  track.show();
}

void playWinnerFx(int carIdx) {
  for (int i = 0; i < (TRACK_PIXELS > 30 ? 30 : TRACK_PIXELS); ++i) track.setPixelColor(i, CAR_COLORS[carIdx]);
  track.show();
  int sz = sizeof(WIN_MUSIC) / sizeof(int);
  for (int note = 0; note < sz; ++note) {
    if (WIN_MUSIC[note] > 0) tone(PIN_AUDIO, WIN_MUSIC[note], 200);
    delay(200); noTone(PIN_AUDIO);
  }
  winnerIdx = carIdx;
}

void updatePhysics() {
  for (int i = 0; i < NUM_CARS; ++i) {
    bool pressed = (digitalRead(PIN_CAR[i]) == LOW);
    if (cars[i].accel_ready && pressed) {
      cars[i].speed += ACCELERATION;
      cars[i].accel_ready = false;
    }
    if (!pressed) cars[i].accel_ready = true;
    int pos = static_cast<int>(cars[i].distance) % TRACK_PIXELS;
    if (gravity_map[pos] < GRAVITY_BASE)
      cars[i].speed -= GRAVITY_CONST * (GRAVITY_BASE - gravity_map[pos]);
    else if (gravity_map[pos] > GRAVITY_BASE)
      cars[i].speed += GRAVITY_CONST * (gravity_map[pos] - GRAVITY_BASE);

    cars[i].speed -= cars[i].speed * FRICTION;
    cars[i].distance += cars[i].speed;
    if (cars[i].distance > TRACK_PIXELS * (cars[i].laps + 1)) {
      ++cars[i].laps;
      tone(PIN_AUDIO, 700 + 100 * i);
      tbeep = 3;
    }
  }
}

void checkForWinner() {
  if (winner_announced) return;
  for (int i = 0; i < NUM_CARS; ++i) {
    if (cars[i].laps >= NUM_LAPS) {
      winner_announced = true;
      playWinnerFx(i);
      gameState = RaceState::WINNER;
      winnerShowMillis = millis();
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_CARS; ++i) pinMode(PIN_CAR[i], INPUT_PULLUP);
  pinMode(PIN_START, INPUT_PULLUP);

  track.begin();
  track.setBrightness(128);
  CAR_COLORS[0] = track.Color(255, 0, 0);
  CAR_COLORS[1] = track.Color(0, 255, 0);
  CAR_COLORS[2] = track.Color(0, 0, 255);
  CAR_COLORS[3] = track.Color(255, 255, 255);
  CAR_COLORS[4] = track.Color(255, 255, 0);

  for (int i = 0; i < TRACK_PIXELS; ++i) gravity_map[i] = GRAVITY_BASE;
  matrix.begin();
}

void loop() {
  // Check for DEMO mode exit in any DEMO state (hold START for 2 seconds)
  if (gameState == RaceState::DEMO_RACING || gameState == RaceState::DEMO_WINNER) {
    bool btnPressed = debounceStartButton();
    
    if (btnPressed && !startWasPressed) {
      startPressedTime = millis();
      startWasPressed = true;
    }
    
    if (btnPressed && startWasPressed && (millis() - startPressedTime >= 2000)) {
      // Exit DEMO mode and perform hardware reset
      matrix.displayClear();
      matrix.displayText("   EXIT DEMO - RESET...   ", PA_CENTER, 20, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      
      // Display message with timeout
      unsigned long msgStart = millis();
      while (!matrix.displayAnimate() && (millis() - msgStart < 2000)) {}
      delay(500);
      
      // Perform Arduino hardware reset using watchdog timer
      performSafeReset();
    }
    
    if (!btnPressed) {
      startWasPressed = false;
    }
  }
  
  switch (gameState) {
  case RaceState::WAITING:
    drawBaseTrack(); drawBoxAnimation();
    if (!welcomeMessageSet) {
      matrix.displayText(
        "   Welcome to Led Race 900! Press START >>>        ",
        PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      welcomeMessageSet = true;
    }
    if (matrix.displayAnimate()) matrix.displayReset();
    
    // Check for START button press with debounce
    {
      bool btnPressed = debounceStartButton();
      
      if (btnPressed && !startWasPressed) {
        startPressedTime = millis();
        startWasPressed = true;
      }
      
      // Check if held for 3+ seconds for DEMO mode
      if (btnPressed && startWasPressed && (millis() - startPressedTime >= 3000)) {
        // Enter DEMO mode
        welcomeMessageSet = false;
        matrix.displayClear();
        matrix.displayText("   DEMO MODE ACTIVATED!   ", PA_CENTER, 20, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        
        // Display message with timeout
        unsigned long msgStart = millis();
        while (!matrix.displayAnimate() && (millis() - msgStart < 2000)) {}
        delay(500);
        
        DEMO_resetRace();
        raceCountdown();
        DEMO_active = true;
        gameState = RaceState::DEMO_RACING;
        startWasPressed = false;
      }
      // Normal race start (button released before 3 seconds)
      else if (!btnPressed && startWasPressed && (millis() - startPressedTime < 3000)) {
        welcomeMessageSet = false;
        raceCountdown();
        resetRace();
        gameState = RaceState::RACING;
        startWasPressed = false;
      }
      
      if (!btnPressed) {
        startWasPressed = false;
      }
    }
    break;
    
  case RaceState::DEMO_RACING:
    DEMO_updateVirtualPlayers();
    DEMO_updatePhysics();
    DEMO_checkForWinner();
    DEMO_displayRaceMessage();
    drawBaseTrack(); 
    DEMO_drawCars();
    track.show();
    if (DEMO_tbeep > 0 && --DEMO_tbeep == 0) noTone(PIN_AUDIO);
    delay(ANIM_FRAME_MS);
    break;
    
  case RaceState::DEMO_WINNER:
    DEMO_displayWinnerMessage();
    if (DEMO_winnerShowMillis != 0 && millis() - DEMO_winnerShowMillis > 5000) {
      // Restart DEMO race
      DEMO_resetRace();
      raceCountdown();
      gameState = RaceState::DEMO_RACING;
    }
    break;
    
  case RaceState::RACING:
    updatePhysics();
    checkForWinner();
    displayRaceEssentialMessage();
    drawBaseTrack(); drawCars();
    track.show();
    if (tbeep > 0 && --tbeep == 0) noTone(PIN_AUDIO);
    delay(ANIM_FRAME_MS);
    break;
  case RaceState::WINNER:
    displayWinnerLoopMessage();
    if (winnerShowMillis != 0 && millis() - winnerShowMillis > 5000) {
      computeFinishingOrder();
      lastBlinkMs = 0;
      resultBlinkState = false;
      gameState = RaceState::RESULTS;
    }
    break;
  case RaceState::RESULTS:
    displayWinnerLoopMessage();
    drawRaceResultBox();
    if (digitalRead(PIN_START) == LOW) {
      while (digitalRead(PIN_START) == LOW);
      resetCarIndex = 0;
      resetCarAnimating = false;
      lerpPos = 0.0;
      gameState = RaceState::RESET_CARS;
    }
    delay(ANIM_FRAME_MS);
    break;
  case RaceState::RESET_CARS:
    if (resetCarIndex < NUM_CARS) animateCarReset();
    else {
      resetCarIndex = 0; resetCarAnimating = false; lerpPos = 0.0;
      resetRace();
      raceCountdown();
      gameState = RaceState::RACING;
    }
    delay(ANIM_FRAME_MS);
    break;
  }
}
