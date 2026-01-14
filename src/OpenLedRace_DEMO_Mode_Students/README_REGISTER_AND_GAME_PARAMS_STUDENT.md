# OpenLED Race 900 – STUDENT EDITION  
## Quick Guide: Main Variables & Game Parameters

> **This document helps students and new makers understand and customize the main constants and variables in the OpenLED Race 900 code.**  
> Everything is designed to be easy to learn, modify, and explore. Try changing some parameters, upload the code, and see what happens!

---

## 🚗 Main Game Variables (Registers)

| Name                | What it is                        | What it does / How to use it                | Example value   |
|---------------------|-----------------------------------|---------------------------------------------|----------------|
| `NUM_CARS`          | How many cars can race            | Change it if you want more or fewer players  | `5`            |
| `NUM_LAPS`          | Number of laps to win             | Set a longer or shorter race                | `4`            |
| `TRACK_PIXELS`      | Number of LEDs in your track      | Keep this equal to your NeoPixel strip size  | `900`          |
| `CAR_COLORS[]`      | Color of each car (LED)           | Change to pick your favorite colors!         | see code       |
| `CAR_COLOR_NAMES[]` | Name for each color/car           | Shown on display and for winner messages     | see code       |
| `ACCELERATION`      | How fast cars gain speed          | Higher = more arcade, lower = more realistic | `0.150f`       |
| `FRICTION`          | How quickly cars slow down        | Higher = more braking, lower = slippery      | `0.012f`       |
| `HOLD_TIME_DEMO_ENTRY` | Hold time to enter demo mode   | Change if you want faster/slower access      | `3000` (ms)    |
| `HOLD_TIME_DEMO_EXIT`  | Hold time to exit demo mode    |                                              | `2000` (ms)    |


---

## 🛠️ Pin Assignments (Physical Connections)

| Name            | Arduino Pin | Device connected         |
|-----------------|-------------|--------------------------|
| `PIN_LED_DATA`  | A0          | NeoPixel strip           |
| `PIN_AUDIO`     | 3           | Buzzer or speaker        |
| `PIN_START`     | 8           | "Start" button           |
| `PIN_CAR[]`     | 7,6,5,4,2   | Player buttons (one per car) |
| `LED_MATRIX_CS` | 10          | Matrix display (CS)      |
| `LED_MATRIX_DIN`| 9           | Matrix display (DIN)     |
| `LED_MATRIX_CLK`| 11          | Matrix display (CLK)     |
| `LED_MATRIX_DEV`| 4           | Number of matrix modules |

**Tip:**  
Don’t change these *unless* you change your wiring. If you do, update them here and in the code!

---

## 🎨 Visuals & Animation

| Name                  | What it controls                        | How to modify                       |
|-----------------------|-----------------------------------------|-------------------------------------|
| `ANIM_FRAME_MS`       | Animation frame speed (ms per frame)    | Lower value = smoother/faster race  |
| `COUNTDOWN_STEP_MS`   | "Ready/Set/Go" step duration            | Lower = shorter countdown           |
| `WIN_MUSIC[]`         | Notes for winner jingle                 | You can tweak the melody!           |
| `drawCars()`          | How cars & leader effect are displayed  | Try editing colors or "tail" logic  |
| `drawBaseTrack()`     | The look of the track background        | Change to use different base color  |

---

## 🚦 Tuning the RACE Experience

- For **FAST, dynamic races:**  
  - Increase `ACCELERATION` (e.g. 0.20)
  - Decrease `FRICTION` (e.g. 0.010)

- For **SLOWER, more tactical races:**  
  - Lower `ACCELERATION` (e.g. 0.10)
  - Increase `FRICTION` (e.g. 0.018 or 0.020)

- To change **race length**: adjust `NUM_LAPS`
- To match your **LED strip size**: set `TRACK_PIXELS`  

---

## 🏆 Useful Coding Tips


- Want to add another car?  
  Increase `NUM_CARS`, add a pin to `PIN_CAR[]`, and update colors/names.

- To speed up or slow down the matrix scroll messages,  
  look for this part in the code:  
  `matrix.displayText("yourText", ..., SPEED, ...);`  
  Lower SPEED number = faster scroll.

---

## 💬 Try This!

- Set `ACCELERATION` to `0.25` and see how much faster the game feels!
- Change a car's color or name, e.g., `"ORANGE"` and `track.Color(255, 128, 0)`
- Increase `NUM_LAPS` to make endurance races
- Change the winner tune in `WIN_MUSIC[]` (get creative!)

---

## Need more help?

Read the comments in the `.ino` file — everything is explained step by step!  
If you want to experiment safely, just change **one parameter at a time** and upload the code.

**And remember:**  
> Programming is fun when you experiment and see what happens!

---

Happy racing, and happy hacking! 🚦