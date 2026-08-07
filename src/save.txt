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
const unsigned long GREEN_TIME  = 5000UL; // green duration
const unsigned long YELLOW_TIME = 2000UL; // yellow duration
const unsigned long ALL_RED_TIME = 500UL; // short all-red for safety

// Helper to set three LEDs for a direction
void setLights(uint8_t pinR, uint8_t pinY, uint8_t pinG,
               bool red, bool yellow, bool green) {
  digitalWrite(pinR, red ? HIGH : LOW);
  digitalWrite(pinY, yellow ? HIGH : LOW);
  digitalWrite(pinG, green ? HIGH : LOW);
}

void setup() {
  // configure pins
  pinMode(N_RED, OUTPUT);
  pinMode(N_YELLOW, OUTPUT);
  pinMode(N_GREEN, OUTPUT);

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
  // Phase 1: North & South GREEN, East & West RED
  setLights(N_RED, N_YELLOW, N_GREEN, false, false, true);
  setLights(S_RED, S_YELLOW, S_GREEN, false, false, true);
  setLights(E_RED, E_YELLOW, E_GREEN, true, false, false);
  setLights(W_RED, W_YELLOW, W_GREEN, true, false, false);
  delay(GREEN_TIME);

  // NS YELLOW
  setLights(N_RED, N_YELLOW, N_GREEN, false, true, false);
  setLights(S_RED, S_YELLOW, S_GREEN, false, true, false);
  delay(YELLOW_TIME);

  // All red briefly
  allRed();
  delay(ALL_RED_TIME);

  // Phase 2: East & West GREEN, North & South RED
  setLights(N_RED, N_YELLOW, N_GREEN, true, false, false);
  setLights(S_RED, S_YELLOW, S_GREEN, true, false, false);
  setLights(E_RED, E_YELLOW, E_GREEN, false, false, true);
  setLights(W_RED, W_YELLOW, W_GREEN, false, false, true);
  delay(GREEN_TIME);

  // EW YELLOW
  setLights(E_RED, E_YELLOW, E_GREEN, false, true, false);
  setLights(W_RED, W_YELLOW, W_GREEN, false, true, false);
  delay(YELLOW_TIME);

  // All red briefly
  allRed();
  delay(ALL_RED_TIME);
}
