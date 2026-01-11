# Led-Race900
improvement of the original Open Led Race project by G. Barbov 2019
# Open LED Race

---

## ENGLISH VERSION

**What is this:**  
Open LED Race 900 is a virtual cars race for Neopixel LED strips, based on the OpenLED Race project by G. Barbarov and greatly improved and customized by Marcello Caselli (mcasel98), with the smart help of GitHub Copilot.

**Main features:**
- Welcome animation, "pit box" activity before the race
- Countdown with Ready-Set-Go! lights
- LED Matrix displays leader, lap, FINAL LAP, winner
- "FINAL LAP!!" message always scrolls completely, never blocks the race
- Fully non-blocking logic: race simulation never pauses!
- Podium lights, winner music, and finish flag show
- Automatic return of cars to the starting line before every new race
- **DEMO MODE**: Automated race with virtual players for demonstrations

### DEMO Mode
**Entry**: Hold START button for at least 3 seconds from the main menu  
**Exit**: Hold START button for 2 seconds during demo (performs hardware reset)

**Features**:
- Completely isolated: all variables and routines have `DEMO_` prefix
- Full normal race with all original animations and effects
- Cars controlled by "virtual players" with variable performance
- Simulated press frequency: 2-5 Hz per car, variable every 1-3.5 seconds
- Simulates humans with different performance and fatigue moments
- Real race logic NOT touched or altered
- Robust anti-bounce routine to prevent false triggers

**Credit where due:**
- Original project: G. Barbarov (gbarbarov@singulardevices.com)
  - https://github.com/gbarbarov/led-race
- Logic/race upgrades and new effects: Marcello Caselli (2026)
- Coding guidance and all-around “pit crew”: GitHub Copilot / OpenAI

**License:**  
Distributed under GPLv3, as the original project.

---

**Enjoy racing, unleash your creativity, and share your improvements!**

---

## VERSIONE ITALIANA

**Cos'è:**  
Open LED Race 900 è una gara di “macchinine” virtuali per strip LED Neopixel, ispirata al progetto originale OpenLED Race di G. Barbarov e pesantemente migliorata e personalizzata da Marcello Caselli (mcasel98), con l’assistenza di GitHub Copilot.

**Funzionalità principali:**
- Animazione di benvenuto e attività box prima della partenza
- Countdown con semaforo Ready-Set-Go!
- Messaggi grafici su LED matrix: leader, giro, ultimo giro, vincitore
- ALERT "ULTIMO GIRO!!" sempre completo e senza bloccare la gara
- Logica totalmente non bloccante: la gara non si ferma mai!
- Premiazione podio, musica per il vincitore e animazione bandiera finale
- Riporto automatico delle macchine sulla linea di partenza ad ogni nuova gara
- **MODALITÀ DEMO**: Gara automatica con giocatori virtuali per dimostrazioni

### Modalità DEMO
**Ingresso**: Tenere premuto START per almeno 3 secondi dal menu principale  
**Uscita**: Tenere premuto START per 2 secondi durante la demo (esegue reset hardware)

**Caratteristiche**:
- Completamente isolata: tutte le variabili e routine hanno prefisso `DEMO_`
- Gara normale completa con tutte le animazioni ed effetti originali
- Auto guidate da "giocatori virtuali" con performance variabili
- Frequenza di pressione simulata: 2-5 Hz per auto, variabile ogni 1-3.5 secondi
- Simula umani con momenti di performance e stanchezza diversi
- Logica della gara reale NON toccata né alterata
- Routine anti-rimbalzo robusta per evitare false attivazioni

**Attenzione ai credits:**
- Progetto originale: G. Barbarov (gbarbarov@singulardevices.com)
  - https://github.com/gbarbarov/led-race
- Upgrade logico, effetti e ottimizzazione: Marcello Caselli (2026)
- Consulenza e code design: GitHub Copilot / OpenAI

**Licenza:**  
Distribuito con licenza GPLv3 come progetto originario.


Marcello Caselli & GitHub Copilot  
2026


