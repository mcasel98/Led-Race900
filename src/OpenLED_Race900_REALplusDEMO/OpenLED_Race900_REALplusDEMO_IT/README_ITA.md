# OpenLED_Race900_REALplusDEMO

**Versione definitiva: Gennaio 2026**  
_Auto slot LED: modalità reale e demo random, accelerazione demo x2, compatibile Arduino MEGA, NeoPixel e MAX72xx / MD_Parola_  

---

## Descrizione

OpenLED Race 900 è un sistema slot car LED open source pensato per **Arduino MEGA**.  
Consente gare in modalità **REALE** (auto e pulsanti hardware) e in modalità **DEMO** (simulazione automatica randomizzata).  
Il software offre separazione completa delle logiche, accelerazione della demo personalizzabile, messaggi guidati, sistemi di vincita, effetti LED e suono, countdown e podio su matrice a LED.

---

## **Funzionalità principali**

- **Modalità REALE:**  
  - Ogni auto è controllata da pulsante hardware dedicato.
  - Accensione dei LED in base alla posizione, effetti scia, animazioni di partenza e podio.
  - Countdown a semaforo, giro in tempo reale, vincitore, reset automatico.
  - Visualizzazione messaggi di gara sulla matrice LED.

- **Modalità DEMO:**  
  - Simulazione automatica del comportamento delle auto con click virtuali random.
  - Accelerazione personalizzabile (default x2).
  - Messaggi di ingresso/uscita demo illustrati su display.
  - Vincitore, podio e reset animati come in gara reale.

- **Effetti e animazioni:**  
  - LED della pista che seguono le auto, effetto scia in testa/coda.
  - Effetti sonori ogni passaggio/giro/vittoria (buzzer).

- **Gestione podio:**  
  - Visualizzazione risultati e podio per le prime posizioni, anche in demo.
  - Lampeggio vincitore.

- **Display:**  
  - Messaggi scrollanti per istruzioni, risultati, stato, countdown.

---

## **Come entrare in DEMO Mode**

**Dopo aver premuto START (all’inizio del countdown):**
1. _Tieni premuto il pulsante START per almeno **3 secondi**._
2. Apparirà una sequenza di **messaggi di conferma** (“DEMO MODE >>”, “To exit”, “Hold START for 2 seconds”).
3. Rilascia il pulsante start.
4. La gara entra in **Modalità DEMO**: le auto si muoveranno in modo automatico e casuale.
---

## **Come uscire dalla DEMO Mode**

**Durante la modalità DEMO (in qualsiasi stato):**
1. _Premi e tieni premuto il pulsante START per almeno **2 secondi**._
2. Verrà mostrato il messaggio scrollante “EXITING DEMO >>”.
3. Rilascia il pulsante start.
4. Il sistema ritorna in **modalità REALE** (pronto per una gara vera).
5. Attendi il display “Benvenuto” prima di ricominciare.

---

## **Adattare i parametri dell’esperienza utente**

Puoi modificare questi parametri per personalizzare la tua gara (zona iniziale del file `.ino`):

- **Numero auto:**  
  `constexpr int NUM_CARS = 5;`
- **Numero giri:**  
  `constexpr int NUM_LAPS = 4;`
- **Lunghezza pista LED:**  
  `constexpr int TRACK_PIXELS = 900;`
- **Accelerazioni:**  
  - `ACCELERATION` (gara reale)
  - `ACCELERATION_DEMO` (demo, esempio x2)
- **Attrito:**  
  `FRICTION` (aumenta per rallentare la pista)
- **Countdown:**  
  `COUNTDOWN_STEP_MS` (millisecondi per ogni step semaforo)
- **Animazioni:**  
  `ANIM_FRAME_MS` (tempo per ogni frame di animazione)
- **Durata podio demo:**  
  `DEMO_PODIUM_DURATION_MS`
- **Colori auto:**  
  Modifica `CAR_COLORS[]` e `CAR_COLOR_NAMES[]` per cambiare colori e nomi delle auto.

---

### **Esempio di personalizzazione**

- **Accelerazione demo turbo:**  
  ```cpp
  constexpr float ACCELERATION_DEMO = ACCELERATION * 3.0f;
  ```
- **Countdown più rapido:**  
  ```cpp
  constexpr int COUNTDOWN_STEP_MS = 600;
  ```
- **Gara corta (2 giri):**  
  ```cpp
  constexpr int NUM_LAPS = 2;
  ```

---

## **Hardware richiesto**

- **Arduino MEGA** (consigliato per memoria e pin!)
- Striscia LED NeoPixel WS2812 o compatibile (fino a 900 pixel)
- Matrice LED MAX72xx gestita da MD_Parola/MD_MAX72xx
- Pulsanti digitali per auto e START

---

## **Licenza & contributi**

Progetto Open Source — miglioralo, modificane i parametri, condividi le tue patch su GitHub!

**Autori:**  
Marcello Caselli (mcasel98)  
GitHub Copilot

---

## **Supporto & contatti**

Per domande, consigli, aiuto e collaborazioni scrivi su GitHub  
https://github.com/mcasel98/Led-Race900

---

**Buone gare ed esperimenti con OpenLED Race900! 🚦🏁**