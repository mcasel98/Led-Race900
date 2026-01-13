# OpenLED_Race900_REALplusDEMO

**Final version: January 2026**  
_LED slot car system: dual real and random demo modes, x2 demo acceleration, compatible with Arduino MEGA, NeoPixel, and MAX72xx / MD_Parola_

---

## Description

OpenLED Race 900 is an open source LED slot car system designed for **Arduino MEGA**.  
It features both **REAL mode** (cars and hardware buttons) and **DEMO mode** (automatic randomized simulation).  
The code offers complete separation of real and demo logic, customizable demo acceleration, guided messages, winner management, LED and sound effects, countdown, and podium displayed on the LED matrix.

---

## **Main Features**

- **REAL Mode:**  
  - Each car is controlled by its dedicated hardware button.
  - LED track lights up according to car position, tail effects, start animations, podium effects.
  - Semaphore countdown, real-time lap counting, winner selection, automatic reset.
  - Race messages shown on the LED matrix display.

- **DEMO Mode:**  
  - Automatic simulation of car behavior with virtual random clicks.
  - Customizable acceleration (default x2).
  - Entry/exit demo messages guided on the display.
  - Winner, podium and reset animations as in real races.

- **Effects & Animations:**  
  - Track LEDs follow the cars, tail effects for leader/trailer.
  - Sound effects for each lap, finish, and victory (buzzer).

- **Podium Management:**  
  - Visualization of rankings and podium for the top finishers, in both real and demo mode.
  - Winner blinking effect.

- **Display:**  
  - Scrolling messages for instructions, results, status, countdown.

---

## **How to Enter DEMO Mode**

**After pressing START (at the beginning of the countdown):**
1. _Hold down the START button for at least **3 seconds**._
2. A sequence of confirmation messages will appear (“DEMO MODE >>”, “To exit”, “Hold START for 2 seconds”).
3. Release the START button.
4. The race switches to **DEMO Mode**: cars move automatically and randomly.

---

## **How to Exit DEMO Mode**

**During DEMO mode (in any state):**
1. _Hold down the START button for at least **2 seconds**._
2. The scrolling message “EXITING DEMO >>” will appear.
3. Release the START button.
4. The system returns to **REAL mode** (ready for a new race).
5. Wait for the “Welcome” display before starting again.

---

## **Customizing Your Experience**

You can modify these key parameters to personalize your race (see the top section of the `.ino` file):

- **Number of cars:**  
  `constexpr int NUM_CARS = 5;`
- **Number of laps:**  
  `constexpr int NUM_LAPS = 4;`
- **LED track length:**  
  `constexpr int TRACK_PIXELS = 900;`
- **Acceleration:**  
  - `ACCELERATION` (real race)
  - `ACCELERATION_DEMO` (demo, e.g., x2)
- **Friction:**  
  `FRICTION` (increase for a slower track)
- **Countdown:**  
  `COUNTDOWN_STEP_MS` (milliseconds per semaphore step)
- **Animations:**  
  `ANIM_FRAME_MS` (time per animation frame)
- **Demo podium duration:**  
  `DEMO_PODIUM_DURATION_MS`
- **Car colors:**  
  Change `CAR_COLORS[]` and `CAR_COLOR_NAMES[]` to adjust car colors/names.

---

### **Customization Examples**

- **Turbo demo acceleration:**  
  ```cpp
  constexpr float ACCELERATION_DEMO = ACCELERATION * 3.0f;
  ```
- **Faster countdown:**  
  ```cpp
  constexpr int COUNTDOWN_STEP_MS = 600;
  ```
- **Short race (2 laps):**  
  ```cpp
  constexpr int NUM_LAPS = 2;
  ```

---

## **Required Hardware**

- **Arduino MEGA** (recommended for memory and pin count)
- NeoPixel WS2812 (or compatible) LED strip (up to 900 pixels)
- MAX72xx LED matrix driven by MD_Parola/MD_MAX72xx
- Digital buttons for cars and START

---

## **License & Contributions**

Open Source project — feel free to improve, tweak parameters, and share your patches or bug reports via GitHub!

**Authors:**  
Marcello Caselli (mcasel98)  
GitHub Copilot

---

## **Support & Contacts**

For questions, advice, help or collaborations, write on GitHub
https://github.com/mcasel98/Led-Race900

---

**Enjoy the races and experiments with OpenLED Race900! 🚦🏁**