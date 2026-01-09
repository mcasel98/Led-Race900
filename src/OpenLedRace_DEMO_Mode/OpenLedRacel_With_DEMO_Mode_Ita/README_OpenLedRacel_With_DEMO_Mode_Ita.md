# Open LED Race 900 – README Italiano

## Descrizione del Progetto

**Open LED Race 900** è una gara automobilistica su strip LED NeoPixel, con visualizzazione su matrice LED, effetti sonori, podio, e modalità DEMO automatica per spettacolo.  
Il progetto è facilmente personalizzabile e pensato sia per gioco competitivo che per esibizione.

- Modalità Gara manuale (START avvia la corsa, pulsanti fisici per accelerare le auto)
- Modalità DEMO infinita (partenza automatica, accelerazioni random, loop continuo)
- Effetti luminosi sulle auto, podio virtuale, messaggi scorrevoli su display
- Pulsante START con antirimbalzo hardware e software

---

## Funzioni Principali

### Modalità Gara

- **Premi START** per avviare la gara reale.
- Ogni giocatore accelera la propria auto tramite il proprio pulsante dedicato (vedi i PIN specificati nel codice).
- La posizione sull’anello, la velocità istantanea, i giri e l’ordine di arrivo delle auto sono visualizzati con effetti luminosi e su matrice LED.
- Vince l’auto che completa per prima il numero di giri (`NUM_LAPS`).

### Modalità DEMO

- **Tieni premuto START per almeno 3 secondi nel menu** per attivare la Demo mode.
- Tutte le auto partono automaticamente: le accelerazioni sono gestite dal software in modo casuale e la velocità è maggiorata rispetto alla gara reale.
- Visualizzazione automatica delle fasi di podio e restart continuo.
- Premi START in qualunque momento in DEMO per tornare al menu.

---

## Variabili e Parametri Modificabili

Per modificare il comportamento e la “percezione” della gara e della demo mode, puoi agire sulle seguenti costanti nel file `.ino`:

### Parametri Generali

- **NUM_CARS**  
  Numero di auto in gara (default: 5)

- **NUM_LAPS**  
  Numero di giri da completare da ciascuna auto per vincere la gara (default: 4)

- **TRACK_PIXELS**  
  Lunghezza del circuito (in LED NeoPixel) percorso dalle auto (default: 900)

---

### Dinamica delle Auto

- **ACCELERATION**  
  Incremento di velocità che ciascuna auto riceve ad ogni pressione del proprio pulsante.  
  _Aumenta questo valore per rendere le auto più “nervose” e la gara più dinamica._  
  Default: `0.150f`
  
- **FRICTION**  
  Fattore di attrito che rallenta costantemente tutte le auto.  
  _Aumenta per gare più tecniche, riduci per effetto “slittamento” delle auto._
  Default: `0.012f`

- **GRAVITY_CONST** e **GRAVITY_BASE**    
  Modificano la velocità delle auto per simularne tratti in salita o discesa.  
  Default: `0.003f` e `127`

---

### Modalità DEMO

- **DEMO_ACCEL_FACTOR**  
  Fattore di moltiplicazione della velocità di accelerazione delle auto SOLO in modalità DEMO.  
  Default: `2.0f` (le auto sono il doppio più veloci rispetto alla gara reale)  
  _Modifica per esaltare l’effetto spettacolo delle auto nella demo._

- **Tempo di pressione START per abilitare DEMO**  
  Determina per quanto tempo è necessario tenere premuto START per far partire la demo delle auto:
  ```cpp
  if (startBtnState == LOW && startHeldActive && (millis() - startLastPressed >= 3000)) {
      demoTriggerReady = true;
  }
  ```
  Sostituisci `3000` con il tempo desiderato (in millisecondi; ad es. 5000 per 5 secondi).

- **Attesa Podio tra una demo e la successiva**
  In modalità DEMO, definisce per quanto tempo le auto restano ferme sul podio prima che inizi una nuova gara automatica:
  ```cpp
  if (millis() - demoPodioStart > 5500) { … }
  ```
  Modifica `5500` (ms = 5.5 secondi) secondo la tua preferenza.

---

### Effetti Visuali per le Auto

- **ANIM_FRAME_MS**  
  Tempo tra un frame di animazione e l’altro per la visualizzazione delle auto:  
  Default: `25`ms  
  _Riduci per rendere il movimento delle auto più fluido e veloce; aumenta per effetto “slow motion”._

- **matrix.displayText(..., velocità, ...)**  
  La velocità di scorrimento dei messaggi su matrice LED (usata per classifica, benvenuto, podio, ecc)  
  Terzo parametro numerico:  
  ```cpp
  matrix.displayText("Testo", PA_CENTER, 12, ...);
  ```
  _Valori più bassi (es: `4` o `6`) = scritte scorrevoli più rapide; valori più alti (es: `16`/`28`) = rallentamento maggiore (più leggibile)._

---

## Connessioni Hardware e Colori Auto

- **PIN_LED_DATA**: pin Arduino collegato allo strip NeoPixel per visualizzazione auto.
- **PIN_AUDIO**: pin per il buzzer.
- **PIN_START**: pulsante di START.
- **PIN_CAR[N]**: array di pin per i pulsanti di ciascuna auto.

- **Configurazione colori auto**:
```cpp
CAR_COLORS[0] = track.Color(255, 0, 0);    // ROSSO (auto #1)
CAR_COLORS[1] = track.Color(0, 255, 0);    // VERDE (auto #2)
CAR_COLORS[2] = track.Color(0, 0, 255);    // BLU (auto #3)
CAR_COLORS[3] = track.Color(255, 255, 255);// BIANCO (auto #4)
CAR_COLORS[4] = track.Color(255, 255, 0);  // GIALLO (auto #5)
```
_Modifica gli RGB per cambiare colore alle auto._

---

## Note Finali

- **Antirimbalzo hardware**: Pulsante START sempre filtrato, nessun falso avvio possibile.
- **Demo Mode**: Ideale per presentazioni pubbliche, fiere, esposizioni.
- **Le auto possono essere facilmente personalizzate nei parametri dinamici e visivi**: colori, accelerazione, attrito e regole gara/demo.

---

## Credits & Fork

Progetto originario:  
https://github.com/gbarbarov/led-race  
Fork con effetti demo, antirimbalzo e presentazione auto:  
https://github.com/mcasel98/Led-Race900

---

### Per domande, personalizzazione o suggerimenti: scrivimi pure!