# DEMO Mode - Technical Documentation

## Overview
The DEMO mode is a completely isolated environment that simulates a normal race with virtual players controlling the cars. This mode is designed for demonstrations and showcases, allowing the LED Race 900 to run autonomously without requiring human players.

## Implementation Specifications

### Entry and Exit
- **Entry**: Hold START button for **≥ 3 seconds** from WAITING state
  - Works even without releasing the button (continuous hold detection)
  - Includes debounce routine to prevent false triggers
  
- **Exit**: Hold START button for **2 seconds** during DEMO mode
  - Triggers a hardware reset of the Arduino using `asm volatile ("jmp 0")`
  - Can be activated from any DEMO state (DEMO_RACING or DEMO_WINNER)

### Complete Isolation
All DEMO mode components use the `DEMO_` prefix to ensure complete isolation from the normal race logic:

#### State Variables
```cpp
- DEMO_active           // Demo mode active flag
- DEMO_cars[NUM_CARS]   // Separate car array for demo
- DEMO_tbeep            // Audio beep counter
- DEMO_winnerIdx        // Winner index for demo
- DEMO_winner_announced // Winner announcement flag
- DEMO_winnerShowMillis // Winner display timing
- DEMO_showUltimoGiroActive / DEMO_showFinalLapActive
- DEMO_ultimoGiroShownOnce / DEMO_finalLapShownOnce
- DEMO_ultimoGiroLedStart / DEMO_finalLapLedStart
- DEMO_ultimoGiroLedEffectDone / DEMO_finalLapLedEffectDone
- DEMO_lastWinnerText[80] // Winner message buffer
```

#### Functions
```cpp
- DEMO_resetRace()
- DEMO_updateVirtualPlayers()
- DEMO_updatePhysics()
- DEMO_drawCars()
- DEMO_displayRaceMessage()
- DEMO_displayWinnerMessage()
- DEMO_playWinnerFx()
- DEMO_checkForWinner()
```

#### State Machine States
```cpp
- RaceState::DEMO_RACING  // Active demo race
- RaceState::DEMO_WINNER  // Demo winner display
```

### Virtual Player Simulation

#### Press Frequency
Each virtual player has an independent press frequency ranging from **2 to 5 Hz**:
```cpp
DEMO_players[i].pressFrequency = random(200, 501) / 100.0f; // 2.0-5.0 Hz
```

#### Frequency Variation
The press frequency changes every **1 to 3.5 seconds** to simulate human performance variations:
```cpp
DEMO_players[i].nextFreqChange = random(1000, 3501); // 1000-3500 ms
```

This creates realistic variation simulating:
- Different skill levels between players
- Fatigue and recovery cycles
- Momentary lapses and bursts of concentration

#### Press Timing
Button presses are simulated based on the current frequency:
```cpp
unsigned long pressPeriod = (unsigned long)(1000.0f / DEMO_players[i].pressFrequency);

if (now - DEMO_players[i].lastPressTime >= pressPeriod) {
    if (DEMO_cars[i].accel_ready) {
        DEMO_cars[i].speed += ACCELERATION;
        DEMO_cars[i].accel_ready = false;
    }
    DEMO_players[i].lastPressTime = now;
}
```

### Debounce Implementation

#### Constants
```cpp
constexpr unsigned long DEBOUNCE_MS = 50; // 50ms debounce time
```

#### Variables
```cpp
bool startBtnState = HIGH;          // Current stable state
bool lastStartBtnReading = HIGH;    // Last raw reading
unsigned long lastDebounceTime = 0; // Last state change time
```

#### Logic
```cpp
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
```

### Race Flow in DEMO Mode

1. **WAITING State**: User holds START for 3+ seconds
2. **Transition**: Display "MODALITÀ DEMO ATTIVATA!" / "DEMO MODE ACTIVATED!"
3. **Countdown**: Standard Ready-Set-Go countdown (shared with normal race)
4. **DEMO_RACING**: 
   - Virtual players control cars
   - Full race logic with all animations
   - All display messages (leader, laps, FINAL LAP)
   - Lap completion sounds
5. **DEMO_WINNER**: Winner celebration with music and effects
6. **Loop**: After 5 seconds, automatically restart to countdown (step 3)

### Original Race Logic Preservation

The DEMO mode implementation:
- **Does NOT modify** any original race functions
- **Does NOT alter** any original race variables
- **Does NOT change** any original race logic
- **Copies and prefixes** all necessary routines with `DEMO_`
- **Shares only** the following safe, read-only elements:
  - `drawBaseTrack()` - Track visualization
  - `raceCountdown()` - Countdown sequence
  - `gravity_map[]` - Terrain configuration (read-only)
  - `CAR_COLORS[]` - Color definitions (read-only)
  - Physical constants (ACCELERATION, FRICTION, etc.)
  - Track/LED/Audio hardware interfaces

### Hardware Compatibility

Fully compatible with the expected hardware:
- **Arduino Mega** (or compatible boards)
- **NeoPixel LED Strip** (900 pixels)
- **MAX72xx LED Matrix** (4 modules)
- **Audio Output** (piezo buzzer on pin 3)
- **Button Inputs** (START + 5 car buttons)

### Code Quality

- **Readable**: Clear function names with DEMO_ prefix
- **Robust**: Debounce protection, safe state transitions
- **Maintainable**: Isolated code that doesn't interfere with normal operation
- **Documented**: Inline comments explain key logic points
- **Non-blocking**: Uses millis() timing, no blocking delays in race logic

## Usage Examples

### Entering DEMO Mode
1. Power on the LED Race 900
2. Wait for welcome message
3. Press and hold START button
4. Keep holding for 3+ seconds
5. System displays "DEMO MODE ACTIVATED!"
6. Release button (or keep holding, it doesn't matter)
7. Countdown begins automatically
8. Virtual players race!

### Exiting DEMO Mode
1. During DEMO race or winner display
2. Press and hold START button
3. Keep holding for 2 seconds
4. System displays "EXIT DEMO - RESET..."
5. Arduino performs hardware reset
6. System returns to initial welcome screen

### Normal Race (unchanged)
1. From welcome screen
2. Press START briefly (less than 3 seconds)
3. Release button
4. Normal countdown and race begin
5. Human players control cars

## Performance Considerations

- **Memory**: ~500 bytes additional RAM for DEMO structures
- **Code**: ~2KB additional program memory
- **CPU**: Minimal overhead (random number generation and timing checks)
- **No impact**: Zero performance impact on normal race mode

## Future Enhancements (Ideas)

- Adjustable difficulty levels (faster/slower virtual players)
- Demo race statistics display
- Multiple demo scenarios (sprint vs endurance)
- AI learning from real player patterns
- Configurable demo parameters via serial interface

---

**Implementation Date**: January 2026  
**Author**: GitHub Copilot (for mcasel98)  
**Based on**: OpenLED Race 900 by Marcello Caselli & Original OpenLED Race by G. Barbarov
