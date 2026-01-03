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

constexpr float ACCELERATION     = 0.126f;
constexpr float FRICTION         = 0.012f;
constexpr float GRAVITY_CONST    = 0.003f;
constexpr int   GRAVITY_BASE     = 127;

constexpr int ANIM_FRAME_MS      = 25;
constexpr int COUNTDOWN_STEP_MS  = 1000;

constexpr const char* CAR_COLOR_NAMES[NUM_CARS] = {
  "ROSSO", "VERDE", "BLU", "BIANCO", "GIALLO"
};

enum class RaceState : uint8_t { WAITING, RACING, WINNER, RESULTS, RESET_CARS };

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

// --- PATCH gestione stato messaggio ULTIMO GIRO ---
bool showUltimoGiroActive = false;   // SE true = il matrix è dedicato alla scritta
bool ultimoGiroShownOnce = false;    // SE true = la finestra è già stata passata
unsigned long ultimoGiroLedStart = 0;
bool ultimoGiroLedEffectDone = false;

/*** STATO MATRIX ***/
bool showingLapMsg = false;
int currentLeaderIdx = -1;
char lastWinnerText[80] = "";

/*** GRAFICA BASE ***/
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
  showUltimoGiroActive = false;
  ultimoGiroShownOnce = false;
  ultimoGiroLedStart = 0;
  ultimoGiroLedEffectDone = false;
  lastWinnerText[0]=0;
}

/*** COUNTDOWN CON SEMAFORO ROSSO-GIALLO-VERDE ***/
void raceCountdown() {
  struct {
    const char* text;
    uint32_t color;
    int freq;
  } steps[] = {
    {"   PRONTI   ",       track.Color(255,0,0), 660},
    {"   ATTENTI   ",      track.Color(255,255,0), 880},
    {"   VIA!   ",         track.Color(0,255,0),    1320}
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

/*** RESET ANIMATO CON CODA ***/
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

/*** DISPLAY GARA: "ULTIMO GIRO" SEMPRE COMPLETO, NESSUN BLOCCO LOGICA ***/
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

  // Gestione super robusta "ULTIMO GIRO!!"
  if (!showUltimoGiroActive && !ultimoGiroShownOnce && lapNow == NUM_LAPS-1) {
    showUltimoGiroActive = true;
    ultimoGiroLedStart = millis();
    ultimoGiroLedEffectDone = false;
    matrix.displayClear();
    matrix.displayText("   ULTIMO GIRO!!   ", PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(lastTextSet, "ULTIMO GIRO");
  }
  if (showUltimoGiroActive) {
    // Effetto led come sempre (trial non-bloccante)
    if (!ultimoGiroLedEffectDone) {
      unsigned long animTime = millis() - ultimoGiroLedStart;
      int animStep = animTime / 400;
      uint32_t c = (animStep==1) ? track.Color(180,0,180) : track.Color(220,220,255);
      for (int i=0; i<30 && i<TRACK_PIXELS; ++i) track.setPixelColor(i, c);
      track.show();
      if (animStep >= 2) {
        for (int i=0; i<30 && i<TRACK_PIXELS; ++i) track.setPixelColor(i,0);
        track.show();
        ultimoGiroLedEffectDone = true;
      }
    }
    // Mantieni display matrix SOLO su "ULTIMO GIRO" finché non ha scrollato via tutto!
    if (matrix.displayAnimate()) {
      matrix.displayReset();
      showUltimoGiroActive = false;
      ultimoGiroShownOnce = true;
      lastTextSet[0]=0;
    }
    return;
  }

  // Classico: giro n/4
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
  snprintf(win_msg, sizeof(win_msg), "   %s   VINCE   LA   GARA!      ", CAR_COLOR_NAMES[safeWinnerIdx]);
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
  switch (gameState) {
  case RaceState::WAITING:
    drawBaseTrack(); drawBoxAnimation();
    if (!welcomeMessageSet) {
      matrix.displayText(
        "   Benvenuto su Led Race 900! Premi START >>>        ",
        PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      welcomeMessageSet = true;
    }
    if (matrix.displayAnimate()) matrix.displayReset();
    if (digitalRead(PIN_START) == LOW) {
      while (digitalRead(PIN_START) == LOW);
      welcomeMessageSet = false;
      raceCountdown();
      resetRace();
      gameState = RaceState::RACING;
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