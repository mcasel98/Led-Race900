# Open LED Race 900 – English README

## Project Description

**Open LED Race 900** is a car racing game on a NeoPixel LED strip, with visualization on LED matrix, sound effects, podium celebration, and automatic DEMO mode for shows or exhibitions.  
The project is designed to be easily customized, for both competitive play and public demonstration.

- Manual Race Mode (START begins the race, physical buttons accelerate the cars)
- Infinite DEMO Mode (automatic start, random acceleration for the cars, continuous racing loop)
- Car light effects, virtual podium display, scrolling messages
- Hardware and software debounced START button

---

## Main Features

### Race Mode

- **Press START** to launch the real race.
- Each player accelerates their own car using their dedicated button (see PIN configuration in the code).
- Car position around the track, current speed, lap count, and ranking are displayed with light effects and on the matrix display.
- The car that completes the required number of laps (`NUM_LAPS`) first wins the race.

### DEMO Mode

- **Hold the START button for at least 3 seconds in the menu** to activate Demo mode.
- All cars start automatically: acceleration is handled by the software with random behavior, and car speed is increased compared to race mode.
- Automatic display of podium and race restart, looping indefinitely.
- Press START any time during DEMO to return to the menu.

---

## Variables and Editable Parameters

To change the car's behavior or the perception of the race and demo mode, adjust the following constants in the `.ino` file:

### General Parameters

- **NUM_CARS**  
  Number of racing cars (default: 5)

- **NUM_LAPS**  
  Number of laps to be completed by each car to win (default: 4)

- **TRACK_PIXELS**  
  Circuit length (number of NeoPixel LEDs covered by the cars, default: 900)

---

### Car Dynamics

- **ACCELERATION**  
  Amount of speed added to each car with every button press.  
  _Increase for faster and more “arcade” car behavior._  
  Default: `0.150f`
  
- **FRICTION**  
  Constant resistance that slows all cars.  
  _Increase for more technical races, decrease for cars to "slide" or keep momentum longer._
  Default: `0.012f`

- **GRAVITY_CONST** and **GRAVITY_BASE**    
  Affect car speed to simulate inclines/declines on various track segments.  
  Default: `0.003f` and `127`

---

### DEMO Mode

- **DEMO_ACCEL_FACTOR**  
  Acceleration multiplier for the cars ONLY in DEMO mode.  
  Default: `2.0f` (cars are twice as fast as in normal races)  
  _Change for more intense show effect with demo cars._

- **Time the START button is held to enable DEMO**  
  Sets how long the START button needs to be held to activate DEMO mode:
  ```cpp
  if (startBtnState == LOW && startHeldActive && (millis() - startLastPressed >= 3000)) {
      demoTriggerReady = true;
  }
  ```
  Replace `3000` with your preferred time in milliseconds (ex. 5000 for 5 seconds).

- **Podium Wait Time between demo races**
  In DEMO mode, this is the time cars remain displayed at the podium before the next automatic race starts:
  ```cpp
  if (millis() - demoPodioStart > 5500) { … }
  ```
  Adjust `5500` (ms = 5.5 seconds) as you like.

---

### Visual Effects for Cars

- **ANIM_FRAME_MS**  
  Animation frame delay for car movement display:  
  Default: `25`ms  
  _Decrease for smoother and faster car motion; increase for “slow motion” effect._

- **matrix.displayText(..., speed, ...)**  
  Text scroll speed for messages shown on the matrix display (used for leaderboard, welcome, podium, etc.).  
  The third numeric parameter:
  ```cpp
  matrix.displayText("Text", PA_CENTER, 12, ...);
  ```
  _Lower values (e.g. `4`, `6`) = faster scrolling; higher values (`16`, `28`) = slower, more readable._

---

## Hardware Connections and Car Colors

- **PIN_LED_DATA**: Arduino pin connected to NeoPixel strip for car visualization.
- **PIN_AUDIO**: buzzer pin for sound effects.
- **PIN_START**: START button pin.
- **PIN_CAR[N]**: array of pins for the car acceleration buttons.

- **Car color configuration**:
```cpp
CAR_COLORS[0] = track.Color(255, 0, 0);    // RED (car #1)
CAR_COLORS[1] = track.Color(0, 255, 0);    // GREEN (car #2)
CAR_COLORS[2] = track.Color(0, 0, 255);    // BLUE (car #3)
CAR_COLORS[3] = track.Color(255, 255, 255);// WHITE (car #4)
CAR_COLORS[4] = track.Color(255, 255, 0);  // YELLOW (car #5)
```
_Edit RGB values to change each car's color._

---

## Final Notes

- **Hardware debounce**: START button is software and hardware filtered, so false starts are impossible.
- **Demo Mode**: Perfect for public shows, fairs, exhibitions.
- **Cars can be easily customized in dynamics and visual effects**: color, acceleration, friction, race/demo logic.

---

## Credits & Fork

Original project:  
https://github.com/gbarbarov/led-race  
Enhanced fork with demo features, debounce and car presentation:  
https://github.com/mcasel98/Led-Race900

---

### For questions, customization, or suggestions: feel free to contact!