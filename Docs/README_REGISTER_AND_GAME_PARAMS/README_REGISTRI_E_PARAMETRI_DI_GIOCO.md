# Open LED Race 900 [DRAG] — Registro Variabili & Parametri di Gioco

## Panoramica

Questo documento fornisce una **descrizione dettagliata dei principali "registri" (costanti, variabili, array)** presenti in `OpenLedRace_final_English_FIX_DRAG.ino`, con la spiegazione delle **loro funzioni** nel codice.  
Particolare attenzione è rivolta ai punti in cui intervenire per **modificare la velocità, la sensazione di gara e i feedback visivi**, per rendere semplice la personalizzazione della tua esperienza di gioco.

---

## Principali registri e funzioni

### 1. Gestione Auto

| **Registro/Costante** | **Tipo**                    | **Funzione**                                                                             |
|-----------------------|-----------------------------|------------------------------------------------------------------------------------------|
| `NUM_CARS`            | `constexpr int`             | Numero di auto/giocatori. Modifica per aumentare o ridurre i partecipanti (default: 5).  |
| `NUM_LAPS`            | `constexpr int`             | Numero di giri per vincere; determina la durata della gara (default: 4).                 |
| `Car cars[NUM_CARS]`  | Array struct                | Per ogni auto: posizione, velocità, giri e stato bottone.                                |
| `CAR_COLORS[NUM_CARS]`| `uint32_t[]`                | Colori (RGB, formato NeoPixel) di ogni auto; modificabile per personalizzare le tinte.   |
| `CAR_COLOR_NAMES[NUM_CARS]` | `const char*[]`       | Nomi dei colori auto per messaggi/podio.                                                 |

**Dettaglio struttura Auto:**
```cpp
struct Car {
  float distance;      // Posizione attuale (indice LED, può essere frazionario)
  float speed;         // Velocità corrente
  uint8_t laps;        // Giri completati
  bool accel_ready;    // Gestione antirimbalzo per il tasto accelerazione
};
```

---

### 2. Tuning di Gioco — Velocità & Effetto Visivo

**Parametri su cui agire per modificare velocità percepita, ritmo di gioco ed effetto della gara:**

| **Registro/Costante**        | **Valore di default** | **Funzione / Effetto**                                                                            |
|------------------------------|-----------------------|---------------------------------------------------------------------------------------------------|
| `ACCELERATION`               | `0.150f`              | **Aumenta per accelerazione più rapida.** Diminuisci per gare più “endurance”/realistiche.        |
| `FRICTION`                   | `0.012f`              | **Valori più alti rallentano le auto più velocemente** (più attrito, meno inerzia).               |
| `GRAVITY_CONST` & `GRAVITY_BASE` | `0.003f` / `127`  | Usati per effetti avanzati tipo "pendenza" (non sempre usati nella versione base, sperimentali).  |
| `ANIM_FRAME_MS`              | `25`                  | **Frame rate animazione.** Più basso = animazione più fluida e veloce; più alto = più lenta.      |
| `COUNTDOWN_STEP_MS`          | `1000`                | **Durata di ciascun step nel countdown.** Riduci per un avvio gara più rapido.                    |
| `TRACK_PIXELS`               | `900`                 | **Numero di LED sulla pista.** Più LED = gara più lunga, maggiore dettaglio visivo.               |

**→ CHIAVE DI GIOCO: Per gare più "arcade" aumenta `ACCELERATION` e/o riduci `FRICTION`.  
Per una gara più tattica o lunga, fai il contrario.**

---

### 3. Configurazione Pin

Definisce la mappatura tra Arduino e hardware esterno (strip, pulsanti, buzzer, display):

| **Nome**              | **Valore**            | **Collegato a**                                  |
|-----------------------|-----------------------|--------------------------------------------------|
| `PIN_LED_DATA`        | `A0`                  | Ingresso dati strip NeoPixel (con resistenza)    |
| `PIN_AUDIO`           | `3`                   | Uscita buzzer/speaker                            |
| `PIN_START`           | `8`                   | Pulsante start/reset                             |
| `PIN_CAR[]`           | `{7, 6, 5, 4, 2}`     | Pulsanti giocatore (uno per ogni auto)           |
| `LED_MATRIX_CS`       | `10`                  | Dot matrix display Chip Select                   |
| `LED_MATRIX_DIN`      | `9`                   | Dot matrix Display Data In                       |
| `LED_MATRIX_CLK`      | `11`                  | Dot matrix Display Clock                         |
| `LED_MATRIX_DEV`      | `4`                   | Numero moduli matrice (separato, non modificare) |

_Non modificare questi parametri se non si cambia il cablaggio hardware._

---

### 4. Controllo Effetti Visivi

Variabili e strutture di stato per display matrix, effetto drag, messaggi gara e podio:

| **Registro/Costante** | **Funzione**                                                      |
|-----------------------|-------------------------------------------------------------------|
| `RaceState gameState` | Gestione stato e fasi principali del gioco                        |
| `drawCars()`          | Disegna auto, leader (effetto DRAG) e code colorate               |
| `drawBaseTrack()`     | Colora tutta la strip come sfondo/pista                           |
| `showFinalLapActive`, `finalLapShownOnce` | Gestiscono la visualizzazione “FINAL LAP!!” |
| `displayRaceEssentialMessage()`          | Messaggi di leader/giri su display matrix         |
| `WIN_MUSIC[]`         | Sequenza note del jingle vincitore (modificabile)                 |

---

## **Dove e come modificare la sensazione di gioco**

### **Velocità & Reattività**
- **Parametri principali:**  
  `ACCELERATION` (**alta = auto "scattano"), `FRICTION` (**alta = rallentano più in fretta**)
- **Per gara più arcade:**  
  - Alza `ACCELERATION` (fino a 0.20–0.25)
  - Abbassa `FRICTION` (fino a 0.008–0.010)
- **Per sensazione endurance/tecnica:**  
  - Abbassa `ACCELERATION` (es. 0.12–0.10)
  - Alza `FRICTION` (fino a 0.018–0.020)

### **Giri di gara e lunghezza pista**
- `NUM_LAPS`: **maggiore = gara più lunga**.
- `TRACK_PIXELS`: impostalo in base alla lunghezza reale della strip LED.

### **Velocità messaggi su matrice**
- Per cambiare la velocità di scorrimento messaggi, modifica il terzo parametro in:
  `matrix.displayText(..., velocità, ...);`  
  Valore basso = scorre veloce, alto = più lento.

---

## **Personalizzazione Visiva e di Gameplay**

- **Effetto DRAG** (`drawCars()`):  
  Modifica la soglia in `constexpr float dragSpeed = 2;`.  
  (Più basso = effetto drag anche a velocità minori.)

- **Colori:**  
  Personalizza gli array `CAR_COLORS[]` e `CAR_COLOR_NAMES[]`.

- **Musica vincitore:**  
  Per cambiare la musica del podio, modifica la sequenza di note in `WIN_MUSIC[]`.

---

## Riepilogo rapido

> **Per cambiare il feeling di gara:**
>
> - Modifica `constexpr float ACCELERATION = ...` (alto=veloce, basso=lento)
> - Modifica `constexpr float FRICTION = ...` (alto=più frenante)
> - Imposta `constexpr int NUM_LAPS = ...` per la durata delle gare
> - Regola `constexpr int TRACK_PIXELS = ...` in base alla tua strip
>
> _Prova vari valori e trova il migliore per la tua pista e il tuo gruppo!_

---

**Per altre modifiche avanzate, controlla all’inizio del file .ino i blocchi con `constexpr ...` e gli array principali.  
Per ogni effetto o logica, consulta e modifica le funzioni riportate qui sopra.**

---

Codice originale e forks:  
https://github.com/gbarbarov/led-race  
https://github.com/mcasel98/Led-Race900
