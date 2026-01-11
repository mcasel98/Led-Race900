# DEMO Mode - User Guide

## Quick Start

### Activating DEMO Mode
1. **Power on** your LED Race 900
2. **Wait** for the welcome message on the LED matrix
3. **Press and hold** the START button
4. **Keep holding** for at least **3 seconds**
5. The display will show "DEMO MODE ACTIVATED!" / "MODALITÀ DEMO ATTIVATA!"
6. **You can now release** the button (or keep holding - it doesn't matter)
7. The countdown will begin automatically
8. **Enjoy** watching the virtual players race!

### Exiting DEMO Mode
1. During the DEMO race (or after a winner is shown)
2. **Press and hold** the START button
3. **Keep holding** for **2 seconds**
4. The display will show "EXIT DEMO - RESET..." / "USCITA DEMO - RESET..."
5. The Arduino will reset and return to the welcome screen

### Starting a Normal Race
1. From the welcome screen
2. **Press** the START button
3. **Release quickly** (before 3 seconds)
4. Normal countdown and race begin
5. Players control cars with their buttons

## What is DEMO Mode?

DEMO mode is an **automatic demonstration mode** where virtual AI players control the race cars. It's perfect for:

- **Store displays** - Show off the LED Race 900 without requiring staff
- **Events and exhibitions** - Attract attention with a continuous racing demo
- **Testing** - Verify all race features are working properly
- **Entertainment** - Watch AI players compete when you don't have enough human players

## How It Works

### Virtual Players
Each car is controlled by a "virtual player" that simulates human button pressing:

- **Press Frequency**: Each player presses their button at a rate between **2-5 times per second**
- **Variable Performance**: The press frequency changes every **1-3.5 seconds** to simulate:
  - Different skill levels
  - Fatigue and recovery
  - Momentary concentration lapses
  - Bursts of energy

This creates **realistic racing** where some cars pull ahead, others catch up, and the outcome is unpredictable!

### Full Race Experience
DEMO mode includes **all the same features** as a normal race:

✅ Welcome animation and pit box activity  
✅ Ready-Set-Go countdown with colored lights  
✅ LED matrix showing race leader and lap count  
✅ "FINAL LAP!!" / "ULTIMO GIRO!!" alert with LED effects  
✅ Winner celebration with music and lights  
✅ All audio beeps and tones  
✅ All LED animations and car colors  

### Automatic Loop
After a winner is declared:
1. Winner celebration plays for **5 seconds**
2. System automatically resets
3. Countdown starts again
4. New race begins with fresh random player speeds
5. **Repeats infinitely** until you exit DEMO mode

## Technical Details

### Button Debounce
The START button uses a **50ms hardware debounce** to prevent false triggers from:
- Button bounce (mechanical contacts settling)
- Electrical noise
- Accidental quick taps

This ensures reliable entry and exit from DEMO mode.

### Safe Reset
When exiting DEMO mode, the system uses a **watchdog timer reset**:
- More reliable than direct memory jumps
- Works across different Arduino boards
- Compatible with various bootloader configurations
- Ensures clean system restart

### Complete Isolation
DEMO mode is **completely separate** from normal race mode:
- Uses different variables (all prefixed with `DEMO_`)
- Uses different functions (all prefixed with `DEMO_`)
- **Never interferes** with normal race logic
- Can be added/removed without affecting normal operation

## Troubleshooting

### DEMO mode won't activate
- **Check**: Are you holding START for at least 3 seconds?
- **Check**: Are you starting from the WAITING screen (welcome message)?
- **Try**: Release and press again, counting "one thousand one, one thousand two, one thousand three"

### Can't exit DEMO mode
- **Check**: Are you holding START for at least 2 seconds during a DEMO race?
- **Try**: Hold the button longer (count to 3 to be sure)
- **Last resort**: Power cycle the Arduino

### DEMO cars don't move
- **Check**: All the same wiring/connections as normal mode should be correct
- **Check**: LED strip is powered and working
- **Try**: Exit DEMO and test a normal race - if that works, DEMO should too

### DEMO races always end the same way
- The virtual player speeds are **randomized each race**
- Different outcomes are expected
- If you see the same pattern repeatedly, there may be a random seed issue
  - Power cycle the Arduino to reset the random number generator

## Tips and Tricks

### Best Display Settings
For maximum visual impact in DEMO mode:
- Ensure good **contrast** between LED strip and background
- Position the **LED matrix** where it's clearly visible
- Adjust **room lighting** if needed (slightly dimmed works well)
- Consider a **dark backdrop** behind the LED strip

### Timing Recommendations
The default timings work well for most situations:
- **3 seconds** to enter: Long enough to be intentional, short enough to be convenient
- **2 seconds** to exit: Quick exit when needed
- **5 seconds** winner display: Enough time to see the celebration

### Event Usage
For trade shows, exhibitions, or store displays:
1. Set up the hardware with good visibility
2. Activate DEMO mode
3. The system will run **continuously and automatically**
4. Staff only needs to monitor and reset if needed
5. Exit DEMO at end of day with 2-second hold

## Frequently Asked Questions

**Q: Does DEMO mode use more power than normal mode?**  
A: No, power consumption is identical - same LEDs, same effects, same timing.

**Q: Can I adjust the virtual player speeds?**  
A: Currently set to 2-5 Hz (hardcoded). Modifications would require changing the source code.

**Q: Will DEMO mode wear out my hardware?**  
A: No more than normal racing. LEDs and components are designed for continuous operation.

**Q: Can I use DEMO mode for testing?**  
A: Yes! It's excellent for verifying all features work: LEDs, matrix, audio, animations, etc.

**Q: What happens if power is lost during DEMO mode?**  
A: System returns to welcome screen on power restoration, just like normal operation.

**Q: Can both DEMO and normal modes work in the same session?**  
A: Yes! Switch freely between modes. Exiting DEMO returns to welcome; you can then start a normal race.

## Safety Notes

⚠️ **Continuous Operation**: If running DEMO mode for extended periods (hours):
- Ensure adequate **ventilation** for the Arduino and power supply
- Monitor **temperature** of components
- Check that **power connections** remain secure

⚠️ **LED Brightness**: Long exposure to bright LEDs:
- Consider **brightness adjustment** if needed (requires code modification)
- Provide **viewing distance recommendations** for observers
- Avoid **direct eye exposure** to LEDs at close range

⚠️ **Audio Volume**: If audio is loud in a quiet space:
- Consider **volume adjustment** (may require hardware modification)
- Be mindful of **noise levels** in public spaces

## Credits and Support

**DEMO Mode Implementation**: GitHub Copilot for mcasel98 (January 2026)  
**Based on**: LED Race 900 by Marcello Caselli  
**Original Project**: OpenLED Race by G. Barbarov

**For support**:
- Check the technical documentation: `DEMO_MODE_TECHNICAL.md`
- Check the implementation summary: `DEMO_MODE_IMPLEMENTATION_SUMMARY.md`
- Visit: https://github.com/mcasel98/Led-Race900

---

**Enjoy the show! 🏎️💨**
