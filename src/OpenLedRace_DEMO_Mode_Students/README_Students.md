# OpenLED Race 900 – STUDENT EDITION

Welcome to the **Student/Educational Edition** of OpenLED Race 900!

This project is a fully open-source, Arduino-based slot car track with a long NeoPixel LED strip and a MAX72xx LED matrix for real-time race feedback. It supports both “real” button racing and a “demo” (automatic/random) mode for fun and presentations.

---

## 🏁 What is this project?

- Simulates a racing game with up to 5 cars using real buttons
- Features “DEMO MODE”: autonomous, random races!
- Visual effects, lap counting, winner detection, podium animation
- Ultra-commented code for **learning by reading!**

---

## ⚡️ How to Get Started

1. **Materials Needed**
    - Arduino Mega (or compatible board)
    - NeoPixel LED strip (e.g. WS2812, 900+ pixels for big tracks!)
    - MAX72xx (MAX7219, FC16 type) 8x32 or 4x8 LED matrix display
    - Pushbuttons (1 per car + 1 START)
    - Buzzer (optional)
    - Jumper wires and breadboard
2. **Libraries Needed**
    - Adafruit NeoPixel
    - MD_Parola
    - MD_MAX72xx
    - SPI
    > _All libraries available from the Arduino Library Manager_

3. **Wiring**
    - See the comments in the code for pin assignments and diagrams.
    - NeoPixel Data: `A0` (default)
    - Car Buttons: `7, 6, 5, 4, 2` 
    - Start Button: `8`
    - Matrix Display: `CS=10`, `DIN=9`, `CLK=11` (FC16)
    - Buzzer: `3`

4. **Upload**
    ***** Hardware tested and fully running 01/14/2026    *******
    - Open `OpenLED_Race900_REALplusDEMO_STUDENT.ino` in Arduino IDE.
    - Select your board and COM port.
    - Click upload and have fun!

---

## 📝 How to Use

- **REAL MODE:** Press and hold each car's button to accelerate. Complete the laps before your opponents!
- **Enter DEMO MODE:** Hold the START button for 3 seconds on the main screen.  
    - Watch random cars “race” automatically!  
    - Exit by holding START for 2 seconds.
- **Winner Detection & Podium:** Colorful LED and sound effects celebrate the victor!

---

## 💡 Code Highlights

- All variables, loops, and logic are **fully explained by comments**.
- Functions include: physics simulation, car control, racing state machines, UI animation.
- **Perfect for learning embedded C++, Arduino, and interactive programming!**

---

## 📚 For Teachers

- Use this project to introduce arrays, enums, state machines, and physical computing.
- Explore the physics code: acceleration, friction, gravity.
- Challenge students to add features or modify car colors, lap number, etc.
- Encourage hands-on learning and real hardware debugging!

---

## 📐 Block Diagram

![Block diagram example](docs/block_diagram.jpg)
_See `/docs` folder for editable diagram source if available._

---

## 🏷️ License

This project is released under the [MIT License](LICENSE).

---

## 👥 Credits

- Marcello Caselli (mcasel98)
- GitHub Copilot
- All students and teachers who improve open source!

---

## 🙋‍♂️ Need help?

Open an Issue or pull request in the GitHub repo.  
We encourage discussion, tinkering, and classroom use!

---

**Happy racing, and happy learning!**