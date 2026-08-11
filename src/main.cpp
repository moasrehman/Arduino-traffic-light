#include <Arduino.h>

// Four-way traffic light (3 LEDs per direction)
// Directions: North (N), East (E), South (S), West (W)
// Pin mapping uses D2..D13 (12 pins)

const uint8_t N_RED    = 2;
const uint8_t N_YELLOW = 3;
const uint8_t N_GREEN  = 4;

const uint8_t E_RED    = 5;
const uint8_t E_YELLOW = 6;
const uint8_t E_GREEN  = 7;

const uint8_t S_RED    = 8;
const uint8_t S_YELLOW = 9;
const uint8_t S_GREEN  = 10;

const uint8_t W_RED    = 11;
const uint8_t W_YELLOW = 12;
const uint8_t W_GREEN  = 13;

// Timings (milliseconds)
const unsigned long GREEN_TIME  = 5000UL; // base green duration
const unsigned long YELLOW_TIME = 2000UL; // yellow duration
const unsigned long ALL_RED_TIME = 500UL; // short all-red for safety

// Extension/priority rules
const unsigned long MIN_GREEN = 2000UL;      // minimum green before allowing early cut
const unsigned long EXTEND_SINGLE = 3000UL;  // extension if one side requests
const unsigned long EXTEND_BOTH = 6000UL;    // extension if both reciprocal request

// Request buttons (use A0..A3 as digital inputs)
const uint8_t BTN_N = A0;
const uint8_t BTN_E = A1;
const uint8_t BTN_S = A2;
const uint8_t BTN_W = A3;

// Helper to set three LEDs for a direction
void setLights(uint8_t pinR, uint8_t pinY, uint8_t pinG,
               bool red, bool yellow, bool green) {
  digitalWrite(pinR, red ? HIGH : LOW);
  digitalWrite(pinY, yellow ? HIGH : LOW);
  digitalWrite(pinG, green ? HIGH : LOW);
}

// Requests from physical buttons (sticky until served)
bool reqN = false;
bool reqE = false;
bool reqS = false;
bool reqW = false;

enum Phase { PH_NS_GREEN, PH_NS_YELLOW, PH_ALL_RED, PH_EW_GREEN, PH_EW_YELLOW };
Phase phase = PH_NS_GREEN;
unsigned long phaseStart = 0;
unsigned long phaseDuration = GREEN_TIME;

// Utility to check aggregated requests
bool requestNS() { return reqN || reqS; }
bool requestEW() { return reqE || reqW; }

// Set lamps for all directions based on logical state
void applyPhaseLights(Phase p) {
  switch (p) {
    case PH_NS_GREEN:
      setLights(N_RED, N_YELLOW, N_GREEN, false, false, true);
      setLights(S_RED, S_YELLOW, S_GREEN, false, false, true);
      setLights(E_RED, E_YELLOW, E_GREEN, true, false, false);
      setLights(W_RED, W_YELLOW, W_GREEN, true, false, false);
      break;
    case PH_NS_YELLOW:
      setLights(N_RED, N_YELLOW, N_GREEN, false, true, false);
      setLights(S_RED, S_YELLOW, S_GREEN, false, true, false);
      setLights(E_RED, E_YELLOW, E_GREEN, true, false, false);
      setLights(W_RED, W_YELLOW, W_GREEN, true, false, false);
      break;
    case PH_EW_GREEN:
      setLights(N_RED, N_YELLOW, N_GREEN, true, false, false);
      setLights(S_RED, S_YELLOW, S_GREEN, true, false, false);
      setLights(E_RED, E_YELLOW, E_GREEN, false, false, true);
      setLights(W_RED, W_YELLOW, W_GREEN, false, false, true);
      break;
    case PH_EW_YELLOW:
      setLights(N_RED, N_YELLOW, N_GREEN, true, false, false);
      setLights(S_RED, S_YELLOW, S_GREEN, true, false, false);
      setLights(E_RED, E_YELLOW, E_GREEN, false, true, false);
      setLights(W_RED, W_YELLOW, W_GREEN, false, true, false);
      break;
    case PH_ALL_RED:
    default:
      allRed();
      break;
  }
}

void setup() {
  // configure pins
  pinMode(N_RED, OUTPUT);
  pinMode(N_YELLOW, OUTPUT);
  pinMode(N_GREEN, OUTPUT);

  // buttons
  pinMode(BTN_N, INPUT_PULLUP);
  pinMode(BTN_E, INPUT_PULLUP);
  pinMode(BTN_S, INPUT_PULLUP);
  pinMode(BTN_W, INPUT_PULLUP);

  Serial.begin(9600);

  // initialize phase
  phase = PH_NS_GREEN;
  phaseStart = millis();
  phaseDuration = GREEN_TIME;
  applyPhaseLights(phase);

  pinMode(E_RED, OUTPUT);
  pinMode(E_YELLOW, OUTPUT);
  pinMode(E_GREEN, OUTPUT);

  pinMode(S_RED, OUTPUT);
  pinMode(S_YELLOW, OUTPUT);
  pinMode(S_GREEN, OUTPUT);

  pinMode(W_RED, OUTPUT);
  pinMode(W_YELLOW, OUTPUT);
  pinMode(W_GREEN, OUTPUT);

  // Start with North-South GREEN, East-West RED
  setLights(N_RED, N_YELLOW, N_GREEN, false, false, true);
  setLights(S_RED, S_YELLOW, S_GREEN, false, false, true);

  setLights(E_RED, E_YELLOW, E_GREEN, true, false, false);
  setLights(W_RED, W_YELLOW, W_GREEN, true, false, false);
}

void allRed() {
  setLights(N_RED, N_YELLOW, N_GREEN, true, false, false);
  setLights(E_RED, E_YELLOW, E_GREEN, true, false, false);
  setLights(S_RED, S_YELLOW, S_GREEN, true, false, false);
  setLights(W_RED, W_YELLOW, W_GREEN, true, false, false);
}

void loop() {
  // Poll buttons (active LOW)
  if (digitalRead(BTN_N) == LOW) { reqN = true; Serial.println("Request N"); }
  if (digitalRead(BTN_S) == LOW) { reqS = true; Serial.println("Request S"); }
  if (digitalRead(BTN_E) == LOW) { reqE = true; Serial.println("Request E"); }
  if (digitalRead(BTN_W) == LOW) { reqW = true; Serial.println("Request W"); }

  unsigned long now = millis();

  // handle phase-specific behavior (non-blocking)
  switch (phase) {
    case PH_NS_GREEN: {
      // If NS has requests, optionally extend
      bool req_ns = requestNS();
      bool both_ns = reqN && reqS;
      unsigned long maxDuration = GREEN_TIME + (both_ns ? EXTEND_BOTH : (req_ns ? EXTEND_SINGLE : 0UL));
      if (now - phaseStart >= maxDuration) {
        // move to yellow
        phase = PH_NS_YELLOW;
        phaseStart = now;
        phaseDuration = YELLOW_TIME;
        applyPhaseLights(phase);
      }
      break;
    }
    case PH_NS_YELLOW:
      if (now - phaseStart >= phaseDuration) {
        phase = PH_ALL_RED;
        phaseStart = now;
        phaseDuration = ALL_RED_TIME;
        applyPhaseLights(phase);
      }
      break;
    case PH_ALL_RED:
      if (now - phaseStart >= phaseDuration) {
        // Decide next phase: if EW requested and NS not forcing, go EW; otherwise NS
        if (requestEW()) {
          phase = PH_EW_GREEN;
        } else {
          phase = PH_EW_GREEN; // default alternate to EW
        }
        phaseStart = now;
        phaseDuration = GREEN_TIME;
        applyPhaseLights(phase);
      }
      break;
    case PH_EW_GREEN: {
      // If NS requests while EW green, allow early cut after MIN_GREEN
      if (requestNS() && (now - phaseStart >= MIN_GREEN)) {
        // cut EW short and move to yellow
        phase = PH_EW_YELLOW;
        phaseStart = now;
        phaseDuration = YELLOW_TIME;
        applyPhaseLights(phase);
        break;
      }
      if (now - phaseStart >= phaseDuration) {
        phase = PH_EW_YELLOW;
        phaseStart = now;
        phaseDuration = YELLOW_TIME;
        applyPhaseLights(phase);
      }
      break;
    }
    case PH_EW_YELLOW:
      if (now - phaseStart >= phaseDuration) {
        phase = PH_ALL_RED;
        phaseStart = now;
        phaseDuration = ALL_RED_TIME;
        applyPhaseLights(phase);
        // served requests that correspond to the next green
        if (requestNS()) { reqN = reqS = false; }
        if (requestEW()) { reqE = reqW = false; }
      }
      break;
  }
}
