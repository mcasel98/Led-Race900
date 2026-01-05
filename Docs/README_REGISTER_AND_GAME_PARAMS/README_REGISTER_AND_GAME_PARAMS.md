# Open LED Race 900 [DRAG] — Registers & Game Parameters Reference

## Overview

This document provides a detailed **description of the main "registers" (variables/constants/arrays)** used in `OpenLedRace_final_English_FIX_DRAG.ino`, including their specific **functions** in the code.  
Special attention is given to **where and how to modify game velocity, race perception, and visual feedback**, so you can easily tune the gameplay experience to your needs.

---

## Main Registers and Their Functions

### 1. Car Management

| **Register/Constant** | **Type**                      | **Function**                                                                             |
|-----------------------|-------------------------------|------------------------------------------------------------------------------------------|
| `NUM_CARS`            | `constexpr int`               | Number of cars/players. Change to increase/decrease participants (default: 5).           |
| `NUM_LAPS`            | `constexpr int`               | Number of laps to win; sets the race length (default: 4).                                |
| `Car cars[NUM_CARS]`  | Struct array                  | Stores for each car: position, speed, number of laps, and button status.                 |
| `CAR_COLORS[NUM_CARS]`| `uint32_t[]`                  | RGB color codes (NeoPixel format) for each car; change to modify car colors.             |
| `CAR_COLOR_NAMES[NUM_CARS]` | `const char*[]`         | Array of car color names for display/podium messages.                                    |

**Car Structure Details:**
```cpp
struct Car {
  float distance;      // Current position (LED index, can be fractional)
  float speed;         // Current speed
  uint8_t laps;        // Completed laps
  bool accel_ready;    // Debounces button presses for acceleration
};
```

---

### 2. Game Tuning — Speed & Visual Perception

**Parameters you can change to alter the speed, visual pacing, and sensation of the race:**

| **Register/Constant**        | **Default Value** | **Function / Effect**                                                                                                        |
|------------------------------|-------------------|-----------------------------------------------------------------------------------------------------------------------------|
| `ACCELERATION`               | `0.150f`          | **Increase to make cars accelerate faster.** Lower for a more "endurance/realistic" race.                                    |
| `FRICTION`                   | `0.012f`          | **Higher values slow the cars down more quickly.** (Simulates drag, inertia.)                                                |
| `GRAVITY_CONST` & `GRAVITY_BASE` | `0.003f` / `127` | Used for advanced "track sections" or elevation (not fully used in base version, tweakable for custom tracks).                |
| `ANIM_FRAME_MS`              | `25`              | **Controls the animation frame rate.** Lower = more FPS = smoother/faster animation; higher = slower (more CPU friendly).    |
| `COUNTDOWN_STEP_MS`          | `1000`            | **Duration per step in race countdown.** Reduce for quicker start sequence.                                                  |
| `TRACK_PIXELS`               | `900`             | **LEDs in the track.** More pixels = longer race, more visible detail.                                                       |

**→ KEY: For faster, more dynamic races, increase `ACCELERATION` and/or decrease `FRICTION`.  
For longer, more tactical races, do the opposite.**

---

### 3. Pin Configuration

Defines the physical wiring between Arduino and hardware (strip, buttons, buzzer, display):

| **Name**          | **Value**                  | **Connected To**                        |
|-------------------|---------------------------|------------------------------------------|
| `PIN_LED_DATA`    | `A0`                      | NeoPixel Strip Data Input (via resistor) |
| `PIN_AUDIO`       | `3`                       | Buzzer/speaker output                    |
| `PIN_START`       | `8`                       | Start/Restart pushbutton                 |
| `PIN_CAR[]`       | `{7, 6, 5, 4, 2}`         | Player buttons (one per car)             |
| `LED_MATRIX_CS`   | `10`                      | Dot Matrix display Chip Select           |
| `LED_MATRIX_DIN`  | `9`                       | Dot Matrix display Data In               |
| `LED_MATRIX_CLK`  | `11`                      | Dot Matrix display Clock                 |
| `LED_MATRIX_DEV`  | `4`                       | Number of matrix modules                 |

_Note: Do not change these unless altering the hardware wiring!_

---

### 4. Visual Effects Controls

Structures and variables for matrix display, drag effect, race status, and podium show:

| **Register/Constant** | **Function**                                                |
|-----------------------|-------------------------------------------------------------|
| `RaceState gameState` | Controls the main game mode/state.                          |
| `drawCars()`          | Paints the cars, leader’s DRAG effect, and tails.           |
| `drawBaseTrack()`     | Fills the entire strip as background (track).               |
| `showFinalLapActive`/`finalLapShownOnce` | Manage "FINAL LAP!!" visual event.         |
| `displayRaceEssentialMessage()`        | Shows leader or lap count on the matrix.    |
| `WIN_MUSIC[]`         | Notes for winner jingle (can edit for custom tunes).        |

---

## **Where to Modify for Game Feel and Perception**

### **Speed & Responsiveness**
- **Primary knobs:**  
  `ACCELERATION` (**higher = faster**), `FRICTION` (**higher = more braking**).
- **For a more “arcade” race:**  
  - Increase `ACCELERATION` (up to 0.20-0.25 for ultra-fast).
  - Lower `FRICTION` (down to 0.008 or 0.010).
- **For a more “endurance”/strategic feel:**  
  - Decrease `ACCELERATION` (down to 0.12 or 0.10).
  - Raise `FRICTION` (up to 0.018 or 0.020).

### **Laps and Track Length**
- `NUM_LAPS`: **More laps = longer races.**
- `TRACK_PIXELS`: Set this to your actual LED strip length.

### **Matrix Message Speed**
- To speed up/slow down scrolling messages, adjust the 3rd argument in  
  `matrix.displayText(..., speed, ...);`  
  Lower value = faster scroll.

---

## **Visual/Gameplay Customization**

- **Drag Effect** (`drawCars()`):  
  To adjust the threshold for the leader's drag effect, change `constexpr float dragSpeed = 2;`.
  (Lower = drag effect appears at lower car speeds.)

- **Colors:**  
  `CAR_COLORS[]` and `CAR_COLOR_NAMES[]` arrays can be changed as desired.

- **Winner Music:**  
  To change podium music, edit the `WIN_MUSIC[]` note sequence.

---

## Summary Block

> **Quick changes for race "feel":**
>
> - Edit `constexpr float ACCELERATION = ...`; (higher=faster, lower=slower)
> - Edit `constexpr float FRICTION = ...`; (higher=more realistic braking/drag)
> - Set `constexpr int NUM_LAPS = ...` for longer/shorter matches.
> - Set `constexpr int TRACK_PIXELS = ...` to match your LED strip.
>
> _Test various settings and see what feels best for your group!_

---

**For any further custom logic or parameter, look for `constexpr ...` and arrays at the beginning of the .ino file.  
For in-depth tuning, study and edit functions marked in this documentation.**

---

See original project and fork for upgrades:  
https://github.com/gbarbarov/led-race  
https://github.com/mcasel98/Led-Race900
