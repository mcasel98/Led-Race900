/*  
 *  ______    _   _   _           _       
 * |  ____|  | | | | | |         | |      
 * | |__     | | | | | |   __ _  | |  ___ 
 * |  __|    | | | | | |  / _` | | | / __|
 * | |____   | |_| | | | | (_| | | | \__ \
 * |______|   \___/  |_|  \__,_| |_| |___/
 *
 * OpenLED Race 900 – Real plus Demo Mode
 * --------
# OpenLED Race 900 – Student/Educational Edition

Welcome to **OpenLED Race 900**, the open-source Arduino project that turns LED strips and a MAX72xx matrix into a fully-featured slot car racing game –  
now with a super-commented version for students, makers, and educators!

## Features
- Dual Mode: Real Race (with buttons) & Demo (random, automatic)
- Up to 5 cars, 900+ pixel tracks – fully configurable
- Buzzer and matrix display feedback
- Colorful real-time animations and finish-line special effects
- Heavily commented code ("read me and learn!")  
- MIT License – free to use, share, remix!

## How to Use
1. Install libraries: Adafruit_NeoPixel, MD_Parola, MD_MAX72xx, SPI (all available via Library Manager)
2. Wire your hardware:
    - NeoPixel Data: Pin A0 (changeable)
    - Car Buttons: Pins 7,6,5,4,2 (for each car)
    - Start Button: Pin 8
    - LED Matrix: Pins as set in source
    - Buzzer: Pin 3
3. Upload the provided `OpenLED_Race900_REALplusDEMO_STUDENT.ino` to your Arduino Mega (or compatible board)
4. Have fun, hack, and share your improvements!

## Code Structure

- **Extensive comments** make each function and logic block easy to follow (see file!)
- Block diagram and wiring suggestions included below

## License
Released under the MIT License – see LICENSE file for details.

## Credits
Marcello Caselli (mcasel98)  
Copilot & Contributors  
Arduino community

---

## For Teachers and Learners
Want exercises, customizations, or help using this project in class?  
Open an Issue on GitHub or contact the maintainer – you will get support and materials!
 * Github: https://github.com/mcasel98/OpenLED_Race900

 *
 */

#include <Adafruit_NeoPixel.h>   // Library for controlling NeoPixel LEDs (the track)
#include <MD_Parola.h>           // Library for driving Matrix LED display (texts for race info)
#include <MD_MAX72xx.h>          // Low-level driver for MAX72xx LED matrix chips
#include <SPI.h>                 // Needed for LED Matrix communication

// ---------------------------------------------------------------------------
//                    CONFIGURATION & PIN MAPPING SECTION 
// You set up all the important values for track, LEDs, and buttons here.
// ---------------------------------------------------------------------------

// How many cars (and buttons)? This will create array spaces for their data.
constexpr int NUM_CARS     = 5;     // Number of slot cars on your track

// How many laps for a race? First car to reach this number wins
constexpr int NUM_LAPS     = 4;

// How many LEDs does your track LED strip have? 
constexpr int TRACK_PIXELS = 900;   // Track "pixels": must match your hardware

// Define what pins everything is connected to on the Arduino MEGA
constexpr int PIN_LED_DATA       = A0;              // Pin for LED strip data (use any PWM or digital OK for NeoPixel)
constexpr int PIN_AUDIO          = 3;               // Pin for the buzzer/speaker (audio effects)
constexpr int PIN_START          = 8;               // Pin for the race START button
constexpr int PIN_CAR[NUM_CARS]  = {7, 6, 5, 4, 2}; // Array of pins for car buttons: one for each car

// Matrix LED Display definitions
// These are hardware-specific and may need to be changed for your actual hardware setup!
constexpr int LED_MATRIX_CS      = 10;  // Chip Select (CS) pin for the LED matrix
constexpr int LED_MATRIX_DIN     = 9;   // Data In (DIN) pin for the LED matrix
constexpr int LED_MATRIX_CLK     = 11;  // Clock (CLK) pin for the LED matrix
constexpr int LED_MATRIX_DEV     = 4;   // Number of display modules chained together
constexpr auto LED_MATRIX_HW     = MD_MAX72XX::FC16_HW; // Type of LED matrix -- typically FC16 for widely-used 8x8 MAX7219

// Physics and effects configuration (feel free to experiment)
constexpr float ACCELERATION      = 0.150f;        // How much each button-press accelerates the car
constexpr float ACCELERATION_DEMO = ACCELERATION * 2.0f; // DEMO mode acceleration (twice as fast!)
constexpr float FRICTION          = 0.012f;        // How quickly cars slow down naturally
constexpr float GRAVITY_CONST     = 0.003f;        // How much track "hills" (gravity) slow/speed the car
constexpr int   GRAVITY_BASE      = 127;           // The "flat" gravity value (track is level by default)

// Timings for animation and racing
constexpr int ANIM_FRAME_MS       = 25;            // Delay between animation frames (affects smoothness)
constexpr int COUNTDOWN_STEP_MS   = 1000;          // How long each "READY/SET/GO" step lasts (in ms)
constexpr unsigned long DEMO_PODIUM_DURATION_MS = 4000; // How long to show the podium at the end (demo)

// The car colors, used for LED effects and display messages
constexpr const char* CAR_COLOR_NAMES[NUM_CARS] = {
  "RED", "GREEN", "BLUE", "WHITE", "YELLOW"
};

// ---------------------------------------------------------------------------
//              STRUCTURE FOR STORING CAR STATES
// Each car needs to remember its speed, position, laps, etc.
// ---------------------------------------------------------------------------
enum class RaceState : uint8_t { WAITING, RACING, WINNER, RESULTS, RESET_CARS };
// Above: RaceState is a special type (enum class) for which "phase" the program is in.

// Data about every car. We'll use an array of "Car" structures for this.
// This allows to easily keep track of individual values for all cars!
struct Car {
  float distance    = 0;  // Where it is on the track (LED index, possibly over 900)
  float speed       = 0;  // How "fast" it's moving (in pixels per iteration)
  uint8_t laps      = 0;  // How many laps this car has completed
  bool accel_ready  = true;  // Used for debouncing (so you can't just "hold" the button for infinite acceleration)
};
Car cars[NUM_CARS];  // This gives us an array of all cars!

// ...continues...
// ---------------------------------------------------------------------------
//                        GLOBAL VARIABLES & OBJECTS
// These variables and objects manage the state of the game, LEDs, sound, etc.
// ---------------------------------------------------------------------------

uint8_t gravity_map[TRACK_PIXELS];   // Each "pixel" (LED position) can have its own gravity effect (for hills, etc)
uint32_t CAR_COLORS[NUM_CARS];       // The actual color values for each car (set in setup)
MD_Parola matrix(LED_MATRIX_HW, LED_MATRIX_DIN, LED_MATRIX_CLK, LED_MATRIX_CS, LED_MATRIX_DEV);
// "matrix" is our LED text display object

Adafruit_NeoPixel track(TRACK_PIXELS, PIN_LED_DATA, NEO_GRB + NEO_KHZ800);
// "track" controls the long addressable LED strip

RaceState gameState           = RaceState::WAITING;  // What part of the race are we in right now? Default is WAITING at power-on

char win_msg[80]              = {'\0'};              // Buffer used for winner announcement message
int tbeep                     = 0;                   // Used to manage buzzer timing/durations
bool welcomeMessageSet        = false;               // Has the welcome message been displayed? (avoids repeat)
int winnerIdx                 = -1;                  // Which car won? (-1 = not yet)
bool winner_announced         = false;               // Flag: winner already shown?
unsigned long winnerShowMillis= 0;                   // When did we start showing the winner message?
int finishingOrder[NUM_CARS]  = {0};                 // To keep the final finishing order
unsigned long lastBlinkMs     = 0;                   // Last time the race result blink effect was updated
bool resultBlinkState         = false;               // Is the result box currently "on" or "off" (for blinking)
int resetCarIndex             = 0;                   // Used when resetting all cars for a new race (which one now)
bool resetCarAnimating        = false;               // Are we currently animating the car reset sequence?
float lerpPos                 = 0.0;                 // Animation position for car reset effect
bool showUltimoGiroActive     = false;               // Is "Final Lap" effect active right now?
bool ultimoGiroShownOnce      = false;               // Have we shown "Final Lap" at least once?
unsigned long ultimoGiroLedStart = 0;                // When did the "Final Lap" light effect start?
bool ultimoGiroLedEffectDone  = false;               // Is the final lap light effect finished?
bool showingLapMsg            = false;               // Are we currently showing lap messages?
int currentLeaderIdx          = -1;                  // Index of the current leading car
char lastWinnerText[80]       = "";

// ---------------------------------------------------------------------------
//                  GAME MODE & BUTTON MANAGEMENT
// - Two game modes: REAL_RACE (with physical buttons), DEMO_RACE (automatic/random)
// - Variables to manage button holds/timings, mode switching
// ---------------------------------------------------------------------------

enum class GameMode : uint8_t { REAL_RACE, DEMO_RACE }; // Only two modes for now!
GameMode currentMode = GameMode::REAL_RACE;             // Start in real mode by default

bool startBtnPrev = true;                // Was the start button pressed in the previous loop?
unsigned long startBtnLastChange = 0;    // When did its state last change (used for debouncing)?
unsigned long startBtnDownMs = 0;        // When did we start holding the Start button?
bool startBtnActive = false;             // Is it currently pressed?

// DEMO mode switching helpers
bool wantDemoEntry = false;              // Do we want to ENTER Demo mode? (pressed >3s)
bool wantDemoExit  = false;              // Want to EXIT Demo? (pressed >2s)
unsigned long demoEntryStartMs = 0;      // When did entry press begin
unsigned long demoExitStartMs  = 0;      // When did exit press begin

constexpr unsigned long HOLD_TIME_DEMO_ENTRY = 3000;    // Hold button for 3s to enter DEMO
constexpr unsigned long HOLD_TIME_DEMO_EXIT  = 2000;    // Hold button for 2s to exit DEMO
constexpr unsigned long DEBOUNCE_MS = 50;               // Debounce time for all buttons

// ---------------------------------------------------------------------------
//                  DEMO MODE "VIRTUAL BUTTON" SIMULATORS
// In DEMO, the software controls "fake" buttons for each car, 
// pressing them at random intervals.
// ---------------------------------------------------------------------------
struct VirtualButtonState {
  unsigned long lastPressMs = 0;   // When was last press?
  float pressRateHz = 6.0;         // How many "press events" per second
  unsigned long nextPressDue = 0;  // When is the next scheduled press
  bool isPressed = false;          // Currently being "pressed"?
};
VirtualButtonState demoVirtualButtons[NUM_CARS];
unsigned long lastPerformanceShuffle = 0; // For randomly changing the performance/skill of DEMO cars

// ---------------------------------------------------------------------------
//      FUNCTION DECLARATIONS ("PROTOTYPES")
// In C++ it's a good idea to let the compiler know about all your functions 
// before setup() or loop() uses them. This makes it easier for students
// to see what high-level modules exist.
// ---------------------------------------------------------------------------
void updateStartDebounce();          // Handles debouncing and checks if start button has changed state
unsigned long getStartBtnPressDuration();  // For checking how long the button has been held
void handleDemoModeEntry();          // Handles logic for switching from Real to Demo mode
void handleDemoModeExit();           // Handles logic for switching Demo back to Real
void updateVirtualCarButtons();      // Controls the DEMO-mode button presses
bool isCarButtonActive(int idx);     // Figures out if (real OR virtual) button is active for that car
void loopRealRace();                 // ALL the code for the Real mode (hardware play) goes here
void loopDemoRace();                 // ALL the code for the Demo mode (automatic/random) goes here
void updatePhysicsReal();            // Physics simulation for Real mode
void updatePhysicsDemo();            // Physics for Demo mode (slightly different)
void drawBaseTrack();                // Draws the background/track effect
void drawBoxAnimation();             // Draws the pre-race "activity at the box"
void drawCars();                     // Draw current car positions with colors
void animateCarReset();              // Visually animate cars returning to the start
void resetRace();                    // Resets all cars/variables for a new race
void raceCountdown();                // Ready / Set / Go sequence
void displayRaceEssentialMessage();  // Shows LED matrix messages during race
void displayWinnerLoopMessage();     // Shows winner message & animation
void computeFinishingOrder();        // Computes full race result order
void drawRaceResultBox();            // Shows race result/podium effect
void playWinnerFx(int carIdx);       // Sound/LEDs for winner celebration
void checkForWinner();               // Check if one car has finished enough laps

// ---------------------------------------------------------------------------
//              SETUP() – THE ARDUINO INITIALIZER
// This function only runs ONCE, when the Arduino is first powered up
// or reset. Use this to initialize hardware, libraries, and all variables.
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);  // Start USB serial port (for debugging, view with Serial Monitor)
  
  // Initialize each CAR BUTTON as input (with pull-up resistor)
  for (int i = 0; i < NUM_CARS; ++i) pinMode(PIN_CAR[i], INPUT_PULLUP);
  
  // Start/Stop button also needs INPUT_PULLUP so it's high when not pressed
  pinMode(PIN_START, INPUT_PULLUP);

  // Initialize the NeoPixel LED strip for the track
  track.begin();
  track.setBrightness(128); // 0=off, 255=max
  
  // Assign readable color names to actual NeoPixel values.
  CAR_COLORS[0] = track.Color(255, 0, 0);       // Red
  CAR_COLORS[1] = track.Color(0, 255, 0);       // Green
  CAR_COLORS[2] = track.Color(0, 0, 255);       // Blue
  CAR_COLORS[3] = track.Color(255, 255, 255);   // White
  CAR_COLORS[4] = track.Color(255, 255, 0);     // Yellow
  
  // Create a "flat" gravity map (all track segments start with same gravity)
  for (int i = 0; i < TRACK_PIXELS; ++i) gravity_map[i] = GRAVITY_BASE; // All positions are "flat" at first

  // Initialize the Matrix Display (for scrolling messages, results, etc.)
  matrix.begin();

  // Initialize the random number generator (important for demo mode)
  randomSeed(analogRead(0)); // (optional: analogRead(0) gives random hardware noise)

  // Set up DEMO car button states (all not pressed, default skill=6Hz)
  for (int i = 0; i < NUM_CARS; ++i) {
    demoVirtualButtons[i].lastPressMs = 0;
    demoVirtualButtons[i].pressRateHz = 6.0;
    demoVirtualButtons[i].nextPressDue = 0;
    demoVirtualButtons[i].isPressed = false;
  }
  lastPerformanceShuffle = millis(); // time marker

  // At boot, we're always in WAITING state and REAL RACE mode.
  gameState = RaceState::WAITING;
  currentMode = GameMode::REAL_RACE;
}
// ---------------------------------------------------------------------------
//                  BUTTON HANDLING AND DEBOUNCE LOGIC
// These functions accurately read the start button's state, 
// measure how long it was held down (for mode switching), 
// and manage entry/exit for DEMO mode via long press.
// ---------------------------------------------------------------------------

// This function checks if the Start button state has changed (pressed/released), 
// taking care of "bouncing" (when button flips rapidly for a few ms).
void updateStartDebounce() {
  bool btnVal = digitalRead(PIN_START) == LOW; // true when pressed (because INPUT_PULLUP)
  unsigned long now = millis();
  if (btnVal != startBtnPrev) { // did the state just change?
    if (now - startBtnLastChange > DEBOUNCE_MS) { // debounce: enough ms passed since the last change?
      startBtnPrev = btnVal;                  // update previous state
      startBtnLastChange = now;               // update last change timestamp
      startBtnActive = btnVal;                // set as "actively pressed" if true
      if (btnVal) startBtnDownMs = now;       // when pressed starts, remember the time
    }
  }
  if (!btnVal) startBtnDownMs = 0;            // button released: reset "held down" timer
}

// Returns (in ms) how long the Start button is currently being held down. 
// Returns 0 if not pressed.
unsigned long getStartBtnPressDuration() {
  if (startBtnActive)
    return millis() - startBtnDownMs;
  else
    return 0;
}

// Handles LONG PRESS to enter DEMO mode (when in real mode, while waiting).
void handleDemoModeEntry() {
  if (!startBtnActive) {
    demoEntryStartMs = 0;         // if button released, reset timer
    return;
  }
  if (demoEntryStartMs == 0 && startBtnActive)
    demoEntryStartMs = startBtnDownMs;  // set when press starts
  if (demoEntryStartMs > 0 && millis() - demoEntryStartMs >= HOLD_TIME_DEMO_ENTRY) {
    wantDemoEntry = true;         // flag tells program we want to enter DEMO (held >3s)
  }
}

// Handles LONG PRESS to exit DEMO mode (when in demo mode, while waiting).
void handleDemoModeExit() {
  static bool waitingForRelease = false;
  if (!startBtnActive) {
    demoExitStartMs = 0;          // reset timer
    waitingForRelease = false;
    return;
  }
  if (demoExitStartMs == 0 && startBtnActive)
    demoExitStartMs = startBtnDownMs;
  if (!waitingForRelease &&
      demoExitStartMs > 0 &&
      millis() - demoExitStartMs >= HOLD_TIME_DEMO_EXIT) {
    wantDemoExit = true;          // flag: time to exit DEMO
    waitingForRelease = true;     // Wait until released before acting again
  }
}

// ---------------------------------------------------------------------------
//          DEMO MODE "VIRTUAL BUTTON" STATE LOGIC
// In DEMO mode, the "cars" don't use real buttons, 
// but instead, this code will press their "virtual" button automatically 
// at (random) intervals. This makes a simulated, exciting race.
// ---------------------------------------------------------------------------
void updateVirtualCarButtons() {
  unsigned long now = millis();
  
  // Occasionally, randomize each car's speed to make the "demo" less predictable & more fun!
  if (now - lastPerformanceShuffle > random(1300, 2600)) {
    for (int i = 0; i < NUM_CARS; ++i) {
      float perf = 3.0f + random(0, 30) / 10.0f; // between 3 Hz and 6 Hz
      demoVirtualButtons[i].pressRateHz = perf;   // assign to that car
    }
    lastPerformanceShuffle = now; // reset marker
  }
  for (int i = 0; i < NUM_CARS; ++i) {
    VirtualButtonState& vb = demoVirtualButtons[i];
    if (now >= vb.nextPressDue) {
      vb.isPressed = true;                             // "press"!
      vb.lastPressMs = now;
      unsigned long step = (unsigned long)(1000.0f / vb.pressRateHz); // set how long to next press
      vb.nextPressDue = now + step;
    } else {
      vb.isPressed = false;     // not pressing this cycle
    }
  }
}

// Returns TRUE if the car button *should* be considered "pressed": 
// real input in REAL mode, or software-generated in DEMO.
bool isCarButtonActive(int idx) {
  if (currentMode == GameMode::DEMO_RACE)
    return demoVirtualButtons[idx].isPressed;        // in demo mode, use software "press"
  else
    return digitalRead(PIN_CAR[idx]) == LOW;         // in real mode, use hardware press
}
// ---------------------------------------------------------------------------
//      Function: showDemoEntryMessages()
//
//  When the user holds the START button for the required time to enter DEMO mode,
//  this function shows a sequence of messages on the LED matrix display to explain
//  what to do and how to exit demo mode.
// ---------------------------------------------------------------------------
void showDemoEntryMessages() {
  // 1st message: Indicate that DEMO mode is being entered
  matrix.displayText(
      "   DEMO MODE >>            ",    // Text to show (with some padding for scrolling)
      PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT); // Display parameters: centered, scrolls left
  matrix.displayReset();
  while (!matrix.displayAnimate()) { delay(10); } // Wait until animation finishes
  matrix.displayReset();

  // 2nd message: Brief instruction
  matrix.displayText(
      "   To exit            ",
      PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  matrix.displayReset();
  while (!matrix.displayAnimate()) { delay(10); }
  matrix.displayReset();

  // 3rd message: Detailed instruction to exit demo mode
  matrix.displayText(
      "   Hold START for 2 seconds            ",
      PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  matrix.displayReset();
  while (!matrix.displayAnimate()) { delay(10); }
  matrix.displayReset();
}

// ---------------------------------------------------------------------------
//      MAIN ARDUINO LOOP – where everything happens!
// "loop()" runs repeatedly after setup(). It decides what "mode" you're in
// (real race or demo), manages state transitions, and starts the proper logic.
// ---------------------------------------------------------------------------
void loop() {
  updateStartDebounce(); // Always update button state and debounce

  // Handle switching between modes and state before the race
  if (gameState == RaceState::WAITING) { // Only in the WAITING state!
    if (currentMode == GameMode::REAL_RACE) {
      handleDemoModeEntry();
      if (wantDemoEntry) {   // User held START >3s? Go to DEMO.
        wantDemoEntry = false;
        showDemoEntryMessages();     // Inform the user
        currentMode = GameMode::DEMO_RACE;  // Switch to demo
        gameState = RaceState::WAITING;
        return;                      // Restart loop in new mode
      }
    }
    if (currentMode == GameMode::DEMO_RACE) {
      handleDemoModeExit();
      if (wantDemoExit) {    // User wants to leave demo mode?
        wantDemoExit = false;

        // Show exit message on display
        matrix.displayText(
          "   EXITING DEMO >>            ",
          PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        matrix.displayReset();
        unsigned long showMsgStart = millis();
        while (millis() - showMsgStart < 2000) {
          matrix.displayAnimate();
          delay(10);
        }
        currentMode = GameMode::REAL_RACE;    // Switch back to REAL
        gameState = RaceState::WAITING;
        welcomeMessageSet = false;            // Reset welcome
        // Wait until START button is released to avoid bouncing transitions
        while (digitalRead(PIN_START) == LOW) {
          delay(5);
        }
        return; // Mode switch complete
      }
    }
  }

  // Depending on mode, hand control to appropriate section of the program!
  if (currentMode == GameMode::REAL_RACE)
    loopRealRace();
  else if (currentMode == GameMode::DEMO_RACE)
    loopDemoRace();
}
// ---------------------------------------------------------------------------
//                  REAL RACE LOOP
// This function contains all the logic for the "real" game, 
// where players use real physical buttons.
// The race state transitions from WAITING --> RACING --> WINNER --> RESULTS, etc.
// ---------------------------------------------------------------------------
void loopRealRace() {
  switch (gameState) {
    case RaceState::WAITING:
      drawBaseTrack();    // Draw the empty track background
      drawBoxAnimation(); // Simulate "activity in the pitbox" with random colors
      
      // Show the WELCOME message once at the beginning
      if (!welcomeMessageSet) {
        matrix.displayText(
          "   Welcome to Led Race 900! Press START >>>            ",
          PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        welcomeMessageSet = true; // Don't show again until next WAITING
      }
      if (matrix.displayAnimate()) matrix.displayReset(); // Keep scrolling animation going
      
      // If player presses START (but for less than demo-entry time), begin the race!
      if (startBtnActive && getStartBtnPressDuration() < HOLD_TIME_DEMO_ENTRY) {
        while (digitalRead(PIN_START) == LOW); // Wait until the user releases the button
        welcomeMessageSet = false;             // Reset so message shows next WAITING
        raceCountdown();                       // Do the READY-SET-GO animation
        resetRace();                           // Reset all cars and states
        gameState = RaceState::RACING;         // We'll enter racing phase next cycle
      }
      break;

    case RaceState::RACING:
      handleDemoModeEntry(); // Even DURING a race, you can hold START to go into DEMO mode
      if (wantDemoEntry) {
        wantDemoEntry = false;
        showDemoEntryMessages();
        currentMode = GameMode::DEMO_RACE;
        gameState = RaceState::WAITING;
        return; // Immediately switch to Demo logic
      }
      updatePhysicsReal();      // Move all cars according to physics and button inputs
      checkForWinner();         // Check if someone has finished

      displayRaceEssentialMessage(); // Matrix display: laps, leader, etc.
      drawBaseTrack();              // Draw track
      drawCars();                   // Draw car positions and their lights
      track.show();                 // Update LEDs

      if (tbeep > 0 && --tbeep == 0) noTone(PIN_AUDIO); // Manage sound effect timer

      delay(ANIM_FRAME_MS); // Small delay for smooth animation
      break;

    case RaceState::WINNER:
      // We stay in WINNER state for a fixed time,
      // showing the winning car/message, then move to RESULTS.
      displayWinnerLoopMessage();
      if (winnerShowMillis != 0 && millis() - winnerShowMillis > 5000) { // 5s shown?
        computeFinishingOrder(); // Get full results
        lastBlinkMs = 0;
        resultBlinkState = false;
        gameState = RaceState::RESULTS; // Show podium/results phase next
      }
      break;

    case RaceState::RESULTS:
      // Player can still switch to DEMO even from the podium animation!
      handleDemoModeEntry();
      if (wantDemoEntry) {
        wantDemoEntry = false;
        showDemoEntryMessages();
        currentMode = GameMode::DEMO_RACE;
        gameState = RaceState::WAITING;
        return;
      }
      displayWinnerLoopMessage(); // Keep showing winner
      drawRaceResultBox();        // Draw flashing podium LEDs
      
      // Wait for START to reset cars (short press)
      if (startBtnActive && getStartBtnPressDuration() < HOLD_TIME_DEMO_ENTRY) {
        while (digitalRead(PIN_START) == LOW); // Wait for release
        resetCarIndex = 0;
        resetCarAnimating = false;
        lerpPos = 0.0;
        gameState = RaceState::RESET_CARS;
      }
      delay(ANIM_FRAME_MS);
      break;

    case RaceState::RESET_CARS:
      // This phase visually animates all cars being "brought back to the start"
      if (resetCarIndex < NUM_CARS)
        animateCarReset();
      else {
        // When all cars are back, begin a new race automatically!
        resetCarIndex = 0;
        resetCarAnimating = false;
        lerpPos = 0.0;
        resetRace();
        raceCountdown();
        gameState = RaceState::RACING;
      }
      delay(ANIM_FRAME_MS);
      break;
  }
}
// ---------------------------------------------------------------------------
//                  DEMO RACE LOOP
// This function contains all the logic for DEMO mode, 
// where all cars are controlled by "virtual" random button presses. 
// Great as an automatic display or for testing the visual effects!
// ---------------------------------------------------------------------------
void loopDemoRace() {
  static unsigned long demoRestart = 0; // Static variable: remembers its value between calls

  handleDemoModeExit();   // If the user holds START for >2 sec, we prepare to exit DEMO mode

  if (wantDemoExit) {
    wantDemoExit = false; // Reset flag

    // Animated exit message before leaving demo mode
    matrix.displayText(
      "   EXITING DEMO >>            ",
      PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    matrix.displayReset();
    unsigned long showMsgStart = millis();
    while (millis() - showMsgStart < 2000) {
      matrix.displayAnimate(); // Keep animation running for the exit duration
      delay(10);
    }
    currentMode = GameMode::REAL_RACE;    // Switch mode back to REAL
    gameState = RaceState::WAITING;
    welcomeMessageSet = false;            // Reset welcome message flag

    // Wait for the user to RELEASE the start button before continuing
    while (digitalRead(PIN_START) == LOW) delay(5);
    return;
  }

  updateVirtualCarButtons();  // Simulate button presses for all cars

  switch (gameState) {
    case RaceState::WAITING:
      drawBaseTrack();        // Track background
      drawBoxAnimation();     // Animated box (pre-race pit effect)

      // Show DEMO mode message ONCE
      if (!welcomeMessageSet) {
        matrix.displayText(
          "   DEMO MODE (random)            ",
          PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        welcomeMessageSet = true;
      }
      if (matrix.displayAnimate()) matrix.displayReset();

      // Short (1.4s) pause on welcome before automatically starting racing
      static unsigned long demoAutostart = 0; // Static for persistence
      if (demoAutostart == 0) demoAutostart = millis();
      if (millis() - demoAutostart > 1400) {
        welcomeMessageSet = false;
        raceCountdown();
        resetRace();
        gameState = RaceState::RACING; // Enter racing phase
        demoAutostart = 0;
      }
      break;

    case RaceState::RACING:
      updatePhysicsDemo();      // Move all cars, using DEMO physics (faster/more random)
      checkForWinner();         // Check if someone has finished

      displayRaceEssentialMessage(); // Show laps, leader, etc.
      drawBaseTrack();              // Track background
      drawCars();                   // Draw current car positions on LEDs
      track.show();                 // Actual LED update

      if (tbeep > 0 && --tbeep == 0) noTone(PIN_AUDIO); // Sound effect timing
      
      delay(ANIM_FRAME_MS);         // Animation frame pace

      handleDemoModeExit();         // Check again if we should exit DEMO mode
      if (wantDemoExit) {           // If yes, repeat the exit logic above
        wantDemoExit = false;
        matrix.displayText(
          "   EXITING DEMO >>            ",
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
      // Show winner animation/message for 5s (same logic as real race)
      displayWinnerLoopMessage();
      if (winnerShowMillis != 0 && millis() - winnerShowMillis > 5000) {
        computeFinishingOrder(); // Podium order
        lastBlinkMs = 0;
        resultBlinkState = false;
        gameState = RaceState::RESULTS; // Go to podium animation
      }
      break;

    case RaceState::RESULTS:
      displayWinnerLoopMessage(); // Keep showing winner announcement

      drawRaceResultBox();        // Podium animation

      // After a fixed time, restart DEMO racing automatically (no button needed)
      if (demoRestart == 0) demoRestart = millis();
      if (millis() - demoRestart > DEMO_PODIUM_DURATION_MS) {
        resetCarIndex = 0;
        resetCarAnimating = false;
        lerpPos = 0.0;
        gameState = RaceState::RESET_CARS; // Animate car reset, then race again!
        demoRestart = 0;
      }
      delay(ANIM_FRAME_MS);
      break;

    case RaceState::RESET_CARS:
      demoRestart = 0;

      // Animate each car going back to the start line, one by one
      if (resetCarIndex < NUM_CARS)
        animateCarReset();
      else {
        // When all are ready, start the DEMO race again
        resetCarIndex = 0;
        resetCarAnimating = false;
        lerpPos = 0.0;
        resetRace();
        raceCountdown();
        gameState = RaceState::RACING;
      }
      delay(ANIM_FRAME_MS);
      break;
  }
}
// ---------------------------------------------------------------------------
//          PHYSICS ENGINE – "Moves" the cars around the track
// ---------------------------------------------------------------------------

// This function updates the position, speed, and lap of EVERY car in REAL mode.
// Reads each player's button, applies acceleration, friction and gravity. 
void updatePhysicsReal() {
  for (int i = 0; i < NUM_CARS; ++i) {
    bool pressed = (digitalRead(PIN_CAR[i]) == LOW); // Read hardware button: active LOW

    // "Debounce" logic: accelerate only on new button press, not while holding!
    if (cars[i].accel_ready && pressed) {
      cars[i].speed += ACCELERATION;    // Boost car speed each press
      cars[i].accel_ready = false;      // Wait until button released before allowing another boost
    }
    if (!pressed) cars[i].accel_ready = true; // Button released? ready for next press

    // Simulate "gravity hills": gravity_map can increase or decrease car speed
    int pos = static_cast<int>(cars[i].distance) % TRACK_PIXELS;  // current position on the track (0..899)
    if (gravity_map[pos] < GRAVITY_BASE)
      cars[i].speed -= GRAVITY_CONST * (GRAVITY_BASE - gravity_map[pos]);
    else if (gravity_map[pos] > GRAVITY_BASE)
      cars[i].speed += GRAVITY_CONST * (gravity_map[pos] - GRAVITY_BASE);

    // Apply friction – always slows the car down a little, simulating "drag"
    cars[i].speed -= cars[i].speed * FRICTION;

    // Move the car along the track by its current speed
    cars[i].distance += cars[i].speed;

    // Check if the car has completed a new lap
    if (cars[i].distance > TRACK_PIXELS * (cars[i].laps + 1)) {
      ++cars[i].laps;                          // Increment lap count
      tone(PIN_AUDIO, 700 + 100 * i);          // Play a sound for lap completion (different for each car)
      tbeep = 3;                               // Number of cycles for sound (see elsewhere)
    }
  }
}

// This function is used ONLY in DEMO mode: 
// it automatically moves the cars by simulating button presses at random intervals and uses a higher acceleration.
void updatePhysicsDemo() {
  for (int i = 0; i < NUM_CARS; ++i) {
    bool pressed = isCarButtonActive(i); // Uses *virtual* button logic for demo

    // Same acceleration logic as real, but with a separate (faster) "demo" acceleration
    if (cars[i].accel_ready && pressed) {
      cars[i].speed += ACCELERATION_DEMO;  // Demo cars are "twice as fast" each boost
      cars[i].accel_ready = false;
    }
    if (!pressed) cars[i].accel_ready = true;

    // Gravity and friction are the same as in real mode
    int pos = static_cast<int>(cars[i].distance) % TRACK_PIXELS;
    if (gravity_map[pos] < GRAVITY_BASE)
      cars[i].speed -= GRAVITY_CONST * (GRAVITY_BASE - gravity_map[pos]);
    else if (gravity_map[pos] > GRAVITY_BASE)
      cars[i].speed += GRAVITY_CONST * (gravity_map[pos] - GRAVITY_BASE);

    cars[i].speed -= cars[i].speed * FRICTION;

    cars[i].distance += cars[i].speed;

    // Check for lap completion as usual
    if (cars[i].distance > TRACK_PIXELS * (cars[i].laps + 1)) {
      ++cars[i].laps;
      tone(PIN_AUDIO, 700 + 100 * i);
      tbeep = 3;
    }
  }
}

// ---------------------------------------------------------------------------
//             VISUALS: TRACK AND CARS ON THE LED STRIP
// ---------------------------------------------------------------------------

// Draws the default LED "track" background (can show gravity effect as blue glow)
void drawBaseTrack() {
  for (int i = 0; i < TRACK_PIXELS; ++i)
    track.setPixelColor(i, track.Color(0, 0, (GRAVITY_BASE - gravity_map[i]) / 12)); // Blue shade by gravity
}

// Pre-race box animation: adds some visual variety before racing starts
void drawBoxAnimation() {
  for (int i = 0; i < (TRACK_PIXELS > 20 ? 20 : TRACK_PIXELS); ++i) {
    int b = random(0, 13);
    if (b > 10)
      track.setPixelColor(i, track.Color(random(10, 40), random(10, 40), random(10, 40))); // Flickering colorful lights
    else
      track.setPixelColor(i, 0); // Turn off unlit pixels
  }
  track.show();
  delay(5);
}

// Draws each car as a colored LED at its current track position.
// Adds a "tail" or "trail" effect behind fast cars!
void drawCars() {
  constexpr float sciaSpeed = 2; // Speed threshold for a longer tail

  int order[NUM_CARS];           // For "leaderboards" (not strictly needed for drawing)

  // Sort cars by their distance for possible effects
  for (int i = 0; i < NUM_CARS; ++i) order[i] = i;
  for (int i = 0; i < NUM_CARS - 1; ++i)
    for (int j = 0; j < NUM_CARS - 1 - i; ++j)
      if (cars[order[j]].distance > cars[order[j + 1]].distance) {
        int temp = order[j]; order[j] = order[j + 1]; order[j + 1] = temp;
      }

  for (int p = 0; p < NUM_CARS; ++p) {
    int idx = order[p];
    int pos = static_cast<int>(cars[idx].distance) % TRACK_PIXELS;
    track.setPixelColor(pos, CAR_COLORS[idx]); // Main car “dot”

    // Short tail just behind car
    int tailpos = (pos > 0) ? pos - 1 : TRACK_PIXELS - 1;
    track.setPixelColor(tailpos, CAR_COLORS[idx] & 0x2F2F2F); // Dimmer trail pixel

    // For the two cars in the lead, add longer, more visible trails if going fast!
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
// ---------------------------------------------------------------------------
//                ANIMATE CAR RESET – Bring cars visually to the start
// ---------------------------------------------------------------------------
//  This function animates a single car "moving" back to its starting position.
//  It's called in RESET_CARS state, one car at a time, each with a tail/trail effect.
void animateCarReset() {
  static int startPos = 0;        // Store initial position for this reset
  if (!resetCarAnimating) {
    startPos = static_cast<int>(cars[resetCarIndex].distance) % TRACK_PIXELS;
    lerpPos = 0.0;
    resetCarAnimating = true;
  }
  // Compute step size so animation takes ~20 frames
  float step = (startPos) / 20.0;
  lerpPos += step;
  int newPos = startPos - static_cast<int>(lerpPos);
  if (newPos < 0) newPos = 0;
  int tail = 9;

  // Clear all track LEDs
  for (int i=0; i<TRACK_PIXELS; ++i) track.setPixelColor(i, 0);

  // Draw a short tail behind the resetting car (fades color)
  for (int t=0; t<=tail; ++t) {
    int tailpos = newPos + t;
    if (tailpos >= startPos) break;
    uint8_t fade = 200/(t+2);
    uint32_t color = CAR_COLORS[resetCarIndex];
    uint8_t r = (uint8_t)((color>>16)&0xFF), g = (uint8_t)((color>>8)&0xFF), b = (uint8_t)((color)&0xFF);
    track.setPixelColor(tailpos, track.Color((r*fade)>>8,(g*fade)>>8,(b*fade)>>8));
  }
  // Draw car head (main LED)
  track.setPixelColor(newPos, CAR_COLORS[resetCarIndex]);
  track.show();

  // Animation finished for this car?
  if (lerpPos >= startPos) {
    cars[resetCarIndex].distance = 0;   // Reset position
    lerpPos = 0.0;
    resetCarIndex++;                    // Move to next car
    resetCarAnimating = false;
    delay(200);                         // Short pause before next
  }
}

// ---------------------------------------------------------------------------
//                        RESET RACE STATE
//  Resets all car data, timing, flags, sound, winner info so a new race can start.
// ---------------------------------------------------------------------------
void resetRace() {
  memset(cars, 0, sizeof(cars));           // Zero all car data (distance, speed, laps)
  tbeep = 0;                               // No sound queued
  winnerIdx = -1;                          // No winner yet
  winnerShowMillis = 0;                    // No winner-animation running
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

// ---------------------------------------------------------------------------
//                         "READY–SET–GO!" COUNTDOWN
// Animated countdown before the race begins. LEDs and matrix display animate each phase.
// ---------------------------------------------------------------------------
void raceCountdown() {
  struct {
    const char* text;    // Text to display
    uint32_t color;      // Track LED color to show
    int freq;            // Buzzer pitch
  } steps[] = {
    {"   READY   ",       track.Color(255,0,0), 660},
    {"   SET   ",         track.Color(255,255,0), 880},
    {"   GO!   ",         track.Color(0,255,0),    1320}
  };
  for (int s=0; s<3; ++s) {
    // Show colored block and text for this phase
    for (int i=0; i<50 && i<TRACK_PIXELS; ++i)
      track.setPixelColor(i, steps[s].color);
    for (int i=50; i<TRACK_PIXELS; ++i)
      track.setPixelColor(i, 0);
    track.show();

    matrix.displayClear();
    matrix.displayText(steps[s].text, PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    while (!matrix.displayAnimate()) {}  // Wait until scroll finishes
    matrix.displayReset();

    // Play sound for this phase (distinct for READY, SET, GO)
    tone(PIN_AUDIO, steps[s].freq);
    delay(COUNTDOWN_STEP_MS);  // Hold countdown step
    noTone(PIN_AUDIO);

    // Small LED “flash” after each phase for dramatic effect
    for (int i=0; i<50 && i<TRACK_PIXELS; ++i)
      track.setPixelColor(i, track.Color(220,220,220)); // White flash
    track.show();
    delay(70);
    for (int i=0; i<50 && i<TRACK_PIXELS; ++i)
      track.setPixelColor(i, 0); // Back to off/ready
    track.show();
    delay(130);
  }
}

// ---------------------------------------------------------------------------
//                  DURING RACE: SHOW STATUS MESSAGES
//  This function updates the LED matrix display with essential information:
//    - Lap count, Leader, or "Final Lap"
//  Shows special effects on the LED track as well.
// ---------------------------------------------------------------------------
void displayRaceEssentialMessage() {
  static char currentDisplayBuf[64] = "";
  static char lastTextSet[64] = "";
  static int lastLapShowed = -2;
  int leader_idx = 0, lapNow = 0; float leaderDist = -1;

  // Find which car is leading right now
  for (int i = 0; i < NUM_CARS; ++i) {
    if (cars[i].laps > lapNow || (cars[i].laps == lapNow && cars[i].distance > leaderDist)) {
      leader_idx = i; lapNow = cars[i].laps; leaderDist = cars[i].distance;
    }
  }

  // "FINAL LAP" animation/announcement
  if (!showUltimoGiroActive && !ultimoGiroShownOnce && lapNow == NUM_LAPS-1) {
    showUltimoGiroActive = true;
    ultimoGiroLedStart = millis();
    ultimoGiroLedEffectDone = false;
    matrix.displayClear();
    matrix.displayText("   FINAL LAP!!           ", PA_CENTER, 12, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    strcpy(lastTextSet, "FINAL LAP");
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

  // Show current lap (updates when lap increments)
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

  // By default, show the color and number of the leading car ("1st: RED" etc.)
  snprintf(currentDisplayBuf, sizeof(currentDisplayBuf), "   %s   1st           ", CAR_COLOR_NAMES[leader_idx]);
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

// ---------------------------------------------------------------------------
//                     WINNER ANNOUNCEMENT & PODIUM
//                  Handles winner message and celebration effects
// ---------------------------------------------------------------------------

// Show a flashy congratulation message for the winner
void displayWinnerLoopMessage() {
  int safeWinnerIdx = winnerIdx;
  if (safeWinnerIdx < 0 || safeWinnerIdx >= NUM_CARS) safeWinnerIdx = 0;
  snprintf(win_msg, sizeof(win_msg), "   %s   WINS   THE   RACE!           ", CAR_COLOR_NAMES[safeWinnerIdx]);
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

// Compute each car's placement for the podium effect (by laps first, distance next)
void computeFinishingOrder() {
  struct CarRank { int idx; float laps; float dist; } cr[NUM_CARS];
  for (int i = 0; i < NUM_CARS; ++i) {
    cr[i].idx = i; cr[i].laps = cars[i].laps; cr[i].dist = cars[i].distance;
  }
  // Sort cars by most laps, then most distance
  for (int i = 0; i < NUM_CARS - 1; ++i)
    for (int j = 0; j < NUM_CARS - 1 - i; ++j)
      if (cr[j].laps < cr[j + 1].laps ||
        (cr[j].laps == cr[j + 1].laps && cr[j].dist < cr[j + 1].dist)) {
        CarRank temp = cr[j]; cr[j] = cr[j + 1]; cr[j + 1] = temp;
      }
  for (int i = 0; i < NUM_CARS; ++i) finishingOrder[i] = cr[i].idx;
}

// Podium LED animation: cars light up in order with blinking effect
void drawRaceResultBox() {
  if (millis() - lastBlinkMs > 1000) {  // Blink every 1s
    resultBlinkState = !resultBlinkState;
    lastBlinkMs = millis();
  }
  // Turn off all LEDs
  for (int i = 0; i < TRACK_PIXELS; ++i) track.setPixelColor(i, 0);

  int boxGap = 4; // Distance between each podium "box"
  // For display, we light up only the 20 leftmost LEDs with podium colors (1st to last)
  for (int p = 0; p < NUM_CARS && (p + 1) * boxGap <= 20; ++p) {
    int idx = finishingOrder[NUM_CARS - 1 - p]; // 1st place is rightmost
    uint32_t color = CAR_COLORS[idx];
    if (p == NUM_CARS - 1 && resultBlinkState) color = 0; // Winner blinks!
    for (int k = 0; k < boxGap - 1; ++k)
      track.setPixelColor(p * boxGap + k, color);
    track.setPixelColor(p * boxGap + (boxGap - 1), 0x030303); // Separator pixel: dim
  }
  track.show();
}

// Play a winner celebration jingle and light up LEDs!
void playWinnerFx(int carIdx) {
  for (int i = 0; i < (TRACK_PIXELS > 30 ? 30 : TRACK_PIXELS); ++i) track.setPixelColor(i, CAR_COLORS[carIdx]);
  track.show();

  // A simple "jingle": a short melody
  constexpr int WIN_MUSIC[] = {2637, 2637, 0, 2637, 0, 2093, 2637, 0, 3136};
  int sz = sizeof(WIN_MUSIC) / sizeof(int);
  for (int note = 0; note < sz; ++note) {
    if (WIN_MUSIC[note] > 0) tone(PIN_AUDIO, WIN_MUSIC[note], 200);
    delay(200); noTone(PIN_AUDIO);
  }
  winnerIdx = carIdx;
}

// Called each animation frame to see if any car has won the race.
void checkForWinner() {
  if (winner_announced) return;
  for (int i = 0; i < NUM_CARS; ++i) {
    if (cars[i].laps >= NUM_LAPS) {    // First car to finish all laps is winner!
      winner_announced = true;
      playWinnerFx(i);                 // Show sound/LEDs etc for winner
      gameState = RaceState::WINNER;
      winnerShowMillis = millis();     // Remember the moment for duration logic
      break;
    }
  }
}
/* --- END OF FILE --- */

