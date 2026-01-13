/*  
 * ____                     _      ______ _____    _____
  / __ \                   | |    |  ____|  __ \  |  __ \               
 | |  | |_ __   ___ _ __   | |    | |__  | |  | | | |__) |__ _  ___ ___ 
 | |  | | '_ \ / _ \ '_ \  | |    |  __| | |  | | |  _  // _` |/ __/ _ \
 | |__| | |_) |  __/ | | | | |____| |____| |__| | | | \ \ (_| | (_|  __/
  \____/| .__/ \___|_| |_| |______|______|_____/  |_|  \_\__,_|\___\___|
        | |                                                             
        |_|          
 Open LED Race - DUAL MODE
 Modalità reale + DEMO random, accelerazione demo x2, messaggi ingresso demo in 3 passi.
 Versione definitiva gennaio 2026 - mcasel98 & GitHub Copilot

 * ==============================================================
 *  OpenLED Race 900 - REAL plus DEMO
 *  Versione definitiva Gennaio 2026
 *  Autori: Marcello Caselli (mcasel98) & GitHub Copilot
 *
 *  Progetto Open Source Slot Car LED Track
 *  Modalità duale: reale (hardware) e demo (virtuale random)
 *  Accelerazione demo x2. Messaggi demo in 3 passi.
 *  Hardware compatibile: Adafruit NeoPixel, MD_MAX72xx LED Matrix
 *  Compatibile Arduino IDE 1.x/2.x
 * 
 *  Github: https://github.com/mcasel98/OpenLED_Race900
 *  
 * ==============================================================
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

constexpr float ACCELERATION     = 0.150f;
constexpr float ACCELERATION_DEMO = ACCELERATION * 2.0f; // x2 demo!
constexpr float FRICTION         = 0.012f;
constexpr float GRAVITY_CONST    = 0.003f;
constexpr int   GRAVITY_BASE     = 127;

constexpr int ANIM_FRAME_MS      = 25;
constexpr int COUNTDOWN_STEP_MS  = 1000;
constexpr unsigned long DEMO_PODIUM_DURATION_MS = 4000;

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
char win_msg[80]              = {'\0'};
int tbeep                     = 0;
bool welcomeMessageSet        = false;
int winnerIdx                 = -1;
bool winner_announced         = false;
unsigned long winnerShowMillis= 0;
int finishingOrder[NUM_CARS]  = {0};
unsigned long lastBlinkMs     = 0;
bool resultBlinkState         = false;
int resetCarIndex             = 0;
bool resetCarAnimating        = false;
float lerpPos                 = 0.0;
bool showUltimoGiroActive     = false;
bool ultimoGiroShownOnce      = false;
unsigned long ultimoGiroLedStart = 0;
bool ultimoGiroLedEffectDone  = false;
bool showingLapMsg            = false;
int currentLeaderIdx          = -1;
char lastWinnerText[80]       = "";

enum class GameMode : uint8_t { REAL_RACE, DEMO_RACE };
GameMode currentMode = GameMode::REAL_RACE;
bool startBtnPrev = true;                
unsigned long startBtnLastChange = 0;    
unsigned long startBtnDownMs = 0;        
bool startBtnActive = false;             
bool wantDemoEntry = false;     
bool wantDemoExit  = false;     
unsigned long demoEntryStartMs = 0;
unsigned long demoExitStartMs  = 0;
constexpr unsigned long HOLD_TIME_DEMO_ENTRY = 3000;
constexpr unsigned long HOLD_TIME_DEMO_EXIT  = 2000;
constexpr unsigned long DEBOUNCE_MS = 50;

struct VirtualButtonState {
  unsigned long lastPressMs = 0;
  float pressRateHz = 6.0;         
  unsigned long nextPressDue = 0;
  bool isPressed = false;
};
VirtualButtonState demoVirtualButtons[NUM_CARS];
unsigned long lastPerformanceShuffle = 0;

// -------------------------------------------------------
// Prototipi funzioni
// -------------------------------------------------------
void updateStartDebounce();
unsigned long getStartBtnPressDuration();
void handleDemoModeEntry();
void handleDemoModeExit();
void updateVirtualCarButtons();
bool isCarButtonActive(int idx);
void loopRealRace();
void loopDemoRace();
void updatePhysicsReal();
void updatePhysicsDemo();

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
  randomSeed(analogRead(0));
  for (int i = 0; i < NUM_CARS; ++i) {
    demoVirtualButtons[i].lastPressMs = 0;
    demoVirtualButtons[i].pressRateHz = 6.0;
    demoVirtualButtons[i].nextPressDue = 0;
    demoVirtualButtons[i].isPressed = false;
  }
  lastPerformanceShuffle = millis();
  gameState = RaceState::WAITING;
  currentMode = GameMode::REAL_RACE;
}

void updateStartDebounce() {
  bool btnVal = digitalRead(PIN_START) == LOW;
  unsigned long now = millis();
  if (btnVal != startBtnPrev) {
    if (now - startBtnLastChange > DEBOUNCE_MS) {
      startBtnPrev = btnVal;
      startBtnLastChange = now;
      startBtnActive = btnVal;
      if (btnVal) startBtnDownMs = now;
    }
  }
  if (!btnVal) startBtnDownMs = 0;
}
unsigned long getStartBtnPressDuration() {
  if (startBtnActive)
    return millis() - startBtnDownMs;
  else
    return 0;
}
void handleDemoModeEntry() {
  if (!startBtnActive) {
    demoEntryStartMs = 0;
    return;
  }
  if (demoEntryStartMs == 0 && startBtnActive)
    demoEntryStartMs = startBtnDownMs;
  if (demoEntryStartMs > 0 && millis() - demoEntryStartMs >= HOLD_TIME_DEMO_ENTRY) {
    wantDemoEntry = true;
  }
}
void handleDemoModeExit() {
  static bool waitingForRelease = false;
  if (!startBtnActive) {
    demoExitStartMs = 0;
    waitingForRelease = false;
    return;
  }
  if (demoExitStartMs == 0 && startBtnActive)
    demoExitStartMs = startBtnDownMs;
  if (!waitingForRelease &&
      demoExitStartMs > 0 &&
      millis() - demoExitStartMs >= HOLD_TIME_DEMO_EXIT) {
    wantDemoExit = true;
    waitingForRelease = true;
  }
}
void updateVirtualCarButtons() {
  unsigned long now = millis();
  if (now - lastPerformanceShuffle > random(1300, 2600)) {
    for (int i = 0; i < NUM_CARS; ++i) {
      float perf = 3.0f + random(0, 30) / 10.0f;
      demoVirtualButtons[i].pressRateHz = perf;
    }
    lastPerformanceShuffle = now;
  }
  for (int i = 0; i < NUM_CARS; ++i) {
    VirtualButtonState& vb = demoVirtualButtons[i];
    if (now >= vb.nextPressDue) {
      vb.isPressed = true;
      vb.lastPressMs = now;
      unsigned long step = (unsigned long)(1000.0f / vb.pressRateHz);
      vb.nextPressDue = now + step;
    } else {
      vb.isPressed = false;
    }
  }
}
bool isCarButtonActive(int idx) {
  if (currentMode == GameMode::DEMO_RACE)
    return demoVirtualButtons[idx].isPressed;
  else
    return digitalRead(PIN_CAR[idx]) == LOW;
}

void showDemoEntryMessages() {
  // Primo messaggio
  matrix.displayText(
      "   Modalita' DEMO >>            ",
      PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  matrix.displayReset();
  while (!matrix.displayAnimate()) { delay(10); }
  matrix.displayReset();

  // Secondo messaggio
  matrix.displayText(
      "   Per uscire            ",
      PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  matrix.displayReset();
  while (!matrix.displayAnimate()) { delay(10); }
  matrix.displayReset();

  // Terzo messaggio
  matrix.displayText(
      "   Premi START per 2 secondi            ",
      PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  matrix.displayReset();
  while (!matrix.displayAnimate()) { delay(10); }
  matrix.displayReset();
}

void loop() {
  updateStartDebounce();
  if (gameState == RaceState::WAITING) {
    if (currentMode == GameMode::REAL_RACE) {
      handleDemoModeEntry();
      if (wantDemoEntry) {
        wantDemoEntry = false;
        showDemoEntryMessages();
        currentMode = GameMode::DEMO_RACE;
        gameState = RaceState::WAITING;
        return;
      }
    }
    if (currentMode == GameMode::DEMO_RACE) {
      handleDemoModeExit();
      if (wantDemoExit) {
        wantDemoExit = false;
        matrix.displayText(
          "   USCITA DA DEMO MODE >>            ",
          PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        matrix.displayReset();
        unsigned long showMsgStart = millis();
        while (millis() - showMsgStart < 2000) {
          matrix.displayAnimate();
          delay(10);
        }
        currentMode = GameMode::REAL_RACE;
        gameState = RaceState::WAITING;
        welcomeMessageSet = false;
        while (digitalRead(PIN_START) == LOW) {
          delay(5);
        }
        return;
      }
    }
  }
  if (currentMode == GameMode::REAL_RACE)
    loopRealRace();
  else if (currentMode == GameMode::DEMO_RACE)
    loopDemoRace();
}

void loopRealRace() {
  switch (gameState) {
    case RaceState::WAITING:
      drawBaseTrack(); drawBoxAnimation();
      if (!welcomeMessageSet) {
        matrix.displayText(
          "   Benvenuto su Led Race 900! Premi START >>>            ",
          PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        welcomeMessageSet = true;
      }
      if (matrix.displayAnimate()) matrix.displayReset();
      if (startBtnActive && getStartBtnPressDuration() < HOLD_TIME_DEMO_ENTRY) {
        while (digitalRead(PIN_START) == LOW);
        welcomeMessageSet = false;
        raceCountdown();
        resetRace();
        gameState = RaceState::RACING;
      }
      break;
    case RaceState::RACING:
      handleDemoModeEntry();
      if (wantDemoEntry) {
        wantDemoEntry = false;
        showDemoEntryMessages();
        currentMode = GameMode::DEMO_RACE;
        gameState = RaceState::WAITING;
        return;
      }
      updatePhysicsReal();
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
      handleDemoModeEntry();
      if (wantDemoEntry) {
        wantDemoEntry = false;
        showDemoEntryMessages();
        currentMode = GameMode::DEMO_RACE;
        gameState = RaceState::WAITING;
        return;
      }
      displayWinnerLoopMessage();
      drawRaceResultBox();
      if (startBtnActive && getStartBtnPressDuration() < HOLD_TIME_DEMO_ENTRY) {
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

void loopDemoRace() {
  static unsigned long demoRestart = 0;
  handleDemoModeExit();
  if (wantDemoExit) {
    wantDemoExit = false;
    matrix.displayText(
      "   USCITA DA DEMO MODE >>            ",
      PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    matrix.displayReset();
    unsigned long showMsgStart = millis();
    while (millis() - showMsgStart < 2000) {
      matrix.displayAnimate();
      delay(10);
    }
    currentMode = GameMode::REAL_RACE;
    gameState = RaceState::WAITING;
    welcomeMessageSet = false;
    while (digitalRead(PIN_START) == LOW) delay(5);
    return;
  }
  updateVirtualCarButtons();
  switch (gameState) {
    case RaceState::WAITING:
      drawBaseTrack(); drawBoxAnimation();
      if (!welcomeMessageSet) {
        matrix.displayText(
          "   DEMO MODE (random)            ",
          PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        welcomeMessageSet = true;
      }
      if (matrix.displayAnimate()) matrix.displayReset();
      static unsigned long demoAutostart = 0;
      if (demoAutostart == 0) demoAutostart = millis();
      if (millis() - demoAutostart > 1400) {
        welcomeMessageSet = false;
        raceCountdown();
        resetRace();
        gameState = RaceState::RACING;
        demoAutostart = 0;
      }
      break;
    case RaceState::RACING:
      updatePhysicsDemo();
      checkForWinner();
      displayRaceEssentialMessage();
      drawBaseTrack(); drawCars();
      track.show();
      if (tbeep > 0 && --tbeep == 0) noTone(PIN_AUDIO);
      delay(ANIM_FRAME_MS);
      handleDemoModeExit();
      if (wantDemoExit) {
        wantDemoExit = false;
        matrix.displayText(
          "   USCITA DA DEMO MODE >>            ",
          PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        matrix.displayReset();
        unsigned long showMsgStart = millis();
        while (millis() - showMsgStart < 2000) {
          matrix.displayAnimate();
          delay(10);
        }
        currentMode = GameMode::REAL_RACE;
        gameState = RaceState::WAITING;
        welcomeMessageSet = false;
        while (digitalRead(PIN_START) == LOW) delay(5);
        return;
      }
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
      if (demoRestart == 0) demoRestart = millis();
      if (millis() - demoRestart > DEMO_PODIUM_DURATION_MS) {
        resetCarIndex = 0;
        resetCarAnimating = false;
        lerpPos = 0.0;
        gameState = RaceState::RESET_CARS;
        demoRestart = 0;
      }
      delay(ANIM_FRAME_MS);
      break;
    case RaceState::RESET_CARS:
      demoRestart = 0;
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

void updatePhysicsReal() {
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
void updatePhysicsDemo() {
  for (int i = 0; i < NUM_CARS; ++i) {
    bool pressed = isCarButtonActive(i);
    if (cars[i].accel_ready && pressed) {
      cars[i].speed += ACCELERATION_DEMO;
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
  constexpr float sciaSpeed = 2;
  int order[NUM_CARS];
  for (int i = 0; i < NUM_CARS; ++i) order[i] = i;
  for (int i = 0; i < NUM_CARS - 1; ++i)
    for (int j = 0; j < NUM_CARS - 1 - i; ++j)
      if (cars[order[j]].distance > cars[order[j + 1]].distance) {
        int temp = order[j]; order[j] = order[j + 1]; order[j + 1] = temp;
      }
  for (int p = 0; p < NUM_CARS; ++p) {
    int idx = order[p];
    int pos = static_cast<int>(cars[idx].distance) % TRACK_PIXELS;
    track.setPixelColor(pos, CAR_COLORS[idx]);
    int tailpos = (pos > 0) ? pos - 1 : TRACK_PIXELS - 1;
    track.setPixelColor(tailpos, CAR_COLORS[idx] & 0x2F2F2F);
    if (p == NUM_CARS - 1 && cars[idx].speed > sciaSpeed) {
      int scia1 = (tailpos > 0) ? tailpos - 1 : TRACK_PIXELS - 1;
      int scia2 = (scia1 > 0) ? scia1 - 1 : TRACK_PIXELS - 1;
      uint32_t colScia1 = ((CAR_COLORS[idx] & 0xFCFCFC) >> 2);
      uint32_t colScia2 = ((CAR_COLORS[idx] & 0xF8F8F8) >> 3);
      track.setPixelColor(scia1, colScia1);
      track.setPixelColor(scia2, colScia2);
    }
    else if (p == NUM_CARS - 2 && cars[idx].speed > sciaSpeed) {
      int scia1 = (tailpos > 0) ? tailpos - 1 : TRACK_PIXELS - 1;
      uint32_t colScia1 = ((CAR_COLORS[idx] & 0xFCFCFC) >> 2);
      track.setPixelColor(scia1, colScia1);
    }
  }
}
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
  if (!showUltimoGiroActive && !ultimoGiroShownOnce && lapNow == NUM_LAPS-1) {
    showUltimoGiroActive = true;
    ultimoGiroLedStart = millis();
    ultimoGiroLedEffectDone = false;
    matrix.displayClear();
    matrix.displayText("   ULTIMO GIRO!!           ", PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(lastTextSet, "ULTIMO GIRO");
  }
  if (showUltimoGiroActive) {
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
    if (matrix.displayAnimate()) {
      matrix.displayReset();
      showUltimoGiroActive = false;
      ultimoGiroShownOnce = true;
      lastTextSet[0]=0;
    }
    return;
  }
  if (lapNow != lastLapShowed) {
    snprintf(currentDisplayBuf, sizeof(currentDisplayBuf), "   %d/%d           ", (lapNow < NUM_LAPS ? lapNow+1 : NUM_LAPS), NUM_LAPS);
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
  snprintf(currentDisplayBuf, sizeof(currentDisplayBuf), "   %s   1o           ", CAR_COLOR_NAMES[leader_idx]);
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
  snprintf(win_msg, sizeof(win_msg), "   %s   VINCE   LA   GARA!           ", CAR_COLOR_NAMES[safeWinnerIdx]);
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
  constexpr int WIN_MUSIC[] = {2637, 2637, 0, 2637, 0, 2093, 2637, 0, 3136};
  int sz = sizeof(WIN_MUSIC) / sizeof(int);
  for (int note = 0; note < sz; ++note) {
    if (WIN_MUSIC[note] > 0) tone(PIN_AUDIO, WIN_MUSIC[note], 200);
    delay(200); noTone(PIN_AUDIO);
  }
  winnerIdx = carIdx;
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
/* --- FINE DEL FILE --- */