#include <Mozzi.h>
#include <Oscil.h>
#include <tables/triangle2048_int8.h>

// ----- Pin Definitions -----
// Finger (touch) input pins for the oscillator matrix:
#define RIGHT_INDEX 23
#define RIGHT_MIDDLE 22
#define RIGHT_RING 21
#define RIGHT_PINKY 19

#define LEFT_INDEX 18
#define LEFT_MIDDLE 4
#define LEFT_RING 15
#define LEFT_PINKY 13

// Joystick analog pin definitions for frequency bending:
// Top 8 oscillators (rows 0-1) use the left joystick’s Y,
// Bottom 8 oscillators (rows 2-3) use the right joystick’s Y.
#define JOY_LEFT_Y_PIN 34
#define JOY_RIGHT_Y_PIN 27

// Lower amplitude value for the oscillators.
#define OSC_AMPLITUDE 64

// ----- Oscillator Setup -----
// Arrays for the right-hand and left-hand finger pins (which also select rows and columns).
const uint8_t rightPins[4] = { RIGHT_INDEX, RIGHT_MIDDLE, RIGHT_RING, RIGHT_PINKY };
const uint8_t leftPins[4]  = { LEFT_INDEX, LEFT_MIDDLE, LEFT_RING, LEFT_PINKY };

// A 4x4 frequency table for the 16 oscillators.
// Rows correspond to right-hand fingers (index to pinky)
// Columns correspond to left-hand fingers (index to pinky)
const int freqTable[4][4] = {
  {262, 294, 330, 349},   // e.g., C4, D4, E4, F4
  {392, 440, 494, 523},   // e.g., G4, A4, B4, C5
  {587, 659, 698, 784},   // e.g., D5, E5, F5, G5
  {880, 988, 1046, 1175}  // e.g., A5, B5, C6, D6
};

// Create a 4x4 matrix of oscillators using the triangle waveform.
Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE> oscs[4][4] = {
  { Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA) },

  { Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA) },

  { Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA) },

  { Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA),
    Oscil<TRIANGLE2048_NUM_CELLS, MOZZI_AUDIO_RATE>(TRIANGLE2048_DATA) }
};

// This array holds the target amplitude (0 or OSC_AMPLITUDE) for each oscillator.
uint8_t oscTarget[4][4] = {0};

// This array holds the smoothed amplitude for each oscillator.
uint8_t smoothedAmp[4][4] = {0};

// ----- Finger Input Debug Setup -----
// Define an array for the eight physical finger pins.
// Order: 1=Right Index, 2=Right Middle, 3=Right Ring, 4=Right Pinky,
//        5=Left Index, 6=Left Middle, 7=Left Ring, 8=Left Pinky.
const uint8_t fingerPins[8] = { RIGHT_INDEX, RIGHT_MIDDLE, RIGHT_RING, RIGHT_PINKY,
                                LEFT_INDEX, LEFT_MIDDLE, LEFT_RING, LEFT_PINKY };
// We'll use this array to track changes. Initialize previous state to HIGH (not pressed).
bool prevFingerState[8] = { HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH };

void setup() {
  startMozzi();

  // Set all finger pins as inputs with pullup resistors.
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(rightPins[i], INPUT_PULLUP);
    pinMode(leftPins[i], INPUT_PULLUP);
  }
  // Also set up the debug finger pins.
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(fingerPins[i], INPUT_PULLUP);
  }
  
  // Initialize each oscillator with its corresponding base frequency.
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t j = 0; j < 4; j++) {
      oscs[i][j].setFreq(freqTable[i][j]);
    }
  }
  
  Serial.begin(115200);
}

// ----- Mozzi Control Update -----
// This function is called at the control rate.
void updateControl() {
  // --- Joystick Frequency Shift ---
  // Read the joystick Y values using mozziAnalogRead().
  // (Assuming a range of 0 to 1023, with roughly 512 at center.)
  int leftY = mozziAnalogRead(JOY_LEFT_Y_PIN);
  int rightY = mozziAnalogRead(JOY_RIGHT_Y_PIN);

  // Map deviation from center to a shift percentage:
  // For a full deviation (~512) we want ±6% (i.e., multiplier ranges 0.94 to 1.06).
  float leftShift = ((leftY - 512) / 512.0) * 0.06;   // e.g., +0.06 at max up, -0.06 at max down
  float rightShift = ((rightY - 512) / 512.0) * 0.06;   // same for right joystick

  // Update each oscillator's frequency.
  // Use the left joystick’s shift for the top half (rows 0 and 1)
  // and the right joystick’s shift for the bottom half (rows 2 and 3).
  for (uint8_t i = 0; i < 4; i++) {
    float multiplier = (i < 2) ? (1.0 + leftShift) : (1.0 + rightShift);
    for (uint8_t j = 0; j < 4; j++) {
      // Multiply the base frequency by the computed multiplier.
      int newFreq = freqTable[i][j] * multiplier;
      oscs[i][j].setFreq(newFreq);
    }
  }

  // --- Finger Input Amplitude Control ---
  // For every right-left finger combination, set the target amplitude.
  // (If both corresponding finger pins are pressed (LOW), set amplitude; otherwise 0.)
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t j = 0; j < 4; j++) {
      oscTarget[i][j] = ((digitalRead(rightPins[i]) == LOW) && (digitalRead(leftPins[j]) == LOW))
                          ? OSC_AMPLITUDE : 0;
    }
  }
}

// ----- Mozzi Audio Update -----
// This function is called at the audio rate to produce sound.
AudioOutput updateAudio() {
  int32_t mixedOutput = 0;

  // Smooth each oscillator's amplitude and mix the audio output.
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t j = 0; j < 4; j++) {
      // Simple IIR smoothing: new_value = (old_value * 15 + target) / 16.
      smoothedAmp[i][j] = (smoothedAmp[i][j] * 15 + oscTarget[i][j]) >> 4;
      mixedOutput += (oscs[i][j].next() * smoothedAmp[i][j]);
    }
  }

  mixedOutput /= 16; // Scale the overall output.
  return MonoOutput::from16Bit(mixedOutput);
}

// ----- Main Loop -----
// In addition to calling audioHook(), we also check for finger input changes and print the finger number (1-8) for debugging.
void loop() {
  audioHook();

  // Check each of the eight finger inputs.
  for (uint8_t i = 0; i < 8; i++) {
    bool currentState = (digitalRead(fingerPins[i]) == LOW); // true if pressed
    // If the state has changed from the previous reading...
    if (currentState != prevFingerState[i]) {
      prevFingerState[i] = currentState;  // Update stored state.
      // If the finger is now pressed, print its number (1-8).
      if (currentState) {
        Serial.println(i + 1);
      }
    }
  }
}
