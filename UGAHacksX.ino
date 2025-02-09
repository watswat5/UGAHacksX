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

// Joystick analog and button pin definitions:
// Top 8 oscillators (rows 0-1) use the left joystick’s Y,
// Bottom 8 oscillators (rows 2-3) use the right joystick’s Y.
#define JOY_LEFT_Y_PIN 34
#define JOY_LEFT_BUTTON_PIN 32
#define JOY_RIGHT_Y_PIN 27
#define JOY_RIGHT_BUTTON_PIN 12

// Lower amplitude value for the oscillators in normal synth mode.
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

// This array holds the target amplitude (0 or a set value) for each oscillator.
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


// ----- Simon Says Game Variables and Definitions -----
// Extended state machine includes a gap period and a limited input window.
enum SimonState {
  SIMON_IDLE,       // Normal synth mode.
  SIMON_PLAY_NOTE,  // The synth plays the target note.
  SIMON_GAP,        // A brief silence between note playback and user input.
  SIMON_WAIT_INPUT, // Waiting for the player’s guess (limited to the note duration).
  SIMON_FEEDBACK    // Brief feedback on the guess (correct or incorrect).
};

SimonState simonState = SIMON_IDLE;
uint8_t simonLevel = 1;         // Game level (1-4). Level is selected by holding one of the right-hand finger buttons.
uint8_t simonTargetRow = 0;     // The “correct” row (0-3) chosen for this round.
uint8_t simonTargetCol = 0;     // The “correct” column (0-3) chosen for this round.
bool simonCorrect = false;      // Flag set when the user makes a guess.
unsigned long simonTimer = 0;   // For timing note playback, gap durations, input window, and feedback.

// Global variable to hold the note (and response) duration.
unsigned long noteDurationAllowed = 1000;  // Will be computed based on level.

// Feedback duration remains constant.
const unsigned long SIMON_FEEDBACK_DURATION = 1000;   // 1 second feedback.

// In the Simon game we use “game amplitudes” that depend on the row.
// (For example, level 1 (row 0) plays softly while higher rows play louder.)
const uint8_t gameAmps[4] = { 16, 32, 48, 64 };


// ----- Helper Function: Start a Simon Round -----
// Randomly chooses a target note based on the current simonLevel.
// Allowed rows: level 1 uses row 0, level 2 uses rows 0-1, etc.
void startSimonRound() {
  uint8_t allowedRows = simonLevel; // (level 1 -> allowedRows=1, etc.)
  simonTargetRow = random(allowedRows);   // Random row in [0, simonLevel-1]
  simonTargetCol = random(4);               // Random column in [0, 3]
  simonState = SIMON_PLAY_NOTE;
  
  // Compute note playback duration (and allowed response window) based on level:
  // Level 1: 1000ms, level 2: 800ms, level 3: 600ms, level 4: 400ms.
  noteDurationAllowed = 1000 - (simonLevel - 1) * 200;
  simonTimer = millis() + noteDurationAllowed;  // For playing the note.
  
  Serial.print("Simon target: row ");
  Serial.print(simonTargetRow);
  Serial.print(", col ");
  Serial.println(simonTargetCol);
}


void setup() {
  startMozzi();
  
  // Initialize finger pins as inputs with pullup resistors.
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(rightPins[i], INPUT_PULLUP);
    pinMode(leftPins[i], INPUT_PULLUP);
  }
  // Also set up the debug finger pins.
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(fingerPins[i], INPUT_PULLUP);
  }
  
  // Set up the joystick button pins as inputs with pullup.
  pinMode(JOY_LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(JOY_RIGHT_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize each oscillator with its corresponding base frequency.
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t j = 0; j < 4; j++) {
      oscs[i][j].setFreq(freqTable[i][j]);
    }
  }
  
  Serial.begin(115200);
  
  // Seed the random number generator.
  randomSeed(analogRead(0));
}


// ----- Mozzi Control Update -----
// This function is called at the control rate.
void updateControl() {
  // --- Check for Simon Game Start in Idle Mode ---
  // In SIMON_IDLE (normal synth mode), require that the player holds down one of the right-hand finger buttons
  // (which selects the level; 1 = index, 2 = middle, 3 = ring, 4 = pinky) BEFORE pressing both joystick buttons.
  if (simonState == SIMON_IDLE) {
    bool joyLeftPressed = (digitalRead(JOY_LEFT_BUTTON_PIN) == LOW);
    bool joyRightPressed = (digitalRead(JOY_RIGHT_BUTTON_PIN) == LOW);
    
    int chosenLevel = 0;
    // Check the four right-hand finger buttons.
    for (uint8_t i = 0; i < 4; i++) {
      if (digitalRead(rightPins[i]) == LOW) {  // Button pressed
        chosenLevel = i + 1;  // Level is 1-indexed
        break;
      }
    }
    
    // Only if both joystick buttons are pressed AND a level button is held down...
    if (joyLeftPressed && joyRightPressed && chosenLevel != 0) {
      simonLevel = chosenLevel;
      Serial.print("Starting Simon Says at level ");
      Serial.println(simonLevel);
      startSimonRound();
      return; // Skip normal synth processing while the game is active.
    }
  }
  
  // --- Simon Game Logic ---
  if (simonState != SIMON_IDLE) {
    unsigned long currentMillis = millis();
    
    switch (simonState) {
      case SIMON_PLAY_NOTE:
        {
          // In SIMON_PLAY_NOTE, play the target note.
          for (uint8_t i = 0; i < 4; i++) {
            for (uint8_t j = 0; j < 4; j++) {
              oscTarget[i][j] = 0;
            }
          }
          // Activate only the target oscillator with its game amplitude.
          oscTarget[simonTargetRow][simonTargetCol] = gameAmps[simonTargetRow];
          
          // When note playback time is over, transition to the gap state.
          if (currentMillis >= simonTimer) {
            // Compute gap duration based on level:
            // Level 1: 500ms, level 2: 400ms, level 3: 300ms, level 4: 200ms.
            unsigned long gapDuration = 500 - (simonLevel - 1) * 100;
            simonState = SIMON_GAP;
            simonTimer = currentMillis + gapDuration;
          }
        }
        break;
        
      case SIMON_GAP:
        {
          // In the gap state, silence all oscillators.
          for (uint8_t i = 0; i < 4; i++) {
            for (uint8_t j = 0; j < 4; j++) {
              oscTarget[i][j] = 0;
            }
          }
          // When the gap is over, move to waiting for input.
          if (currentMillis >= simonTimer) {
            simonState = SIMON_WAIT_INPUT;
            // Set the allowed input window equal to the note duration.
            simonTimer = currentMillis + noteDurationAllowed;
          }
        }
        break;
        
      case SIMON_WAIT_INPUT:
        {
          // In SIMON_WAIT_INPUT, wait for the player's guess.
          // If the allowed input time expires, treat it as a failure.
          if (currentMillis >= simonTimer) {
            // Time expired without a valid input.
            simonCorrect = false;
            simonState = SIMON_FEEDBACK;
            simonTimer = currentMillis + SIMON_FEEDBACK_DURATION;
            break;
          }
          
          // Otherwise, check for a pressed right-left finger combination.
          bool inputDetected = false;
          uint8_t inputRow = 255, inputCol = 255;
          for (uint8_t i = 0; i < 4; i++) {
            if (digitalRead(rightPins[i]) == LOW) { // Right-hand button for row i
              for (uint8_t j = 0; j < 4; j++) {
                if (digitalRead(leftPins[j]) == LOW) { // Left-hand button for column j
                  inputDetected = true;
                  inputRow = i;
                  inputCol = j;
                  break;
                }
              }
              if (inputDetected) break;
            }
          }
          
          // If a finger combination is pressed, check if it matches the target.
          if (inputDetected) {
            if (inputRow == simonTargetRow && inputCol == simonTargetCol) {
              simonCorrect = true;
            } else {
              simonCorrect = false;
            }
            simonState = SIMON_FEEDBACK;
            simonTimer = currentMillis + SIMON_FEEDBACK_DURATION;
          }
        }
        break;
        
      case SIMON_FEEDBACK:
        {
          // Provide brief feedback: if correct, replay target note; if not, play a “fail” tone.
          for (uint8_t i = 0; i < 4; i++) {
            for (uint8_t j = 0; j < 4; j++) {
              oscTarget[i][j] = 0;
            }
          }
          if (simonCorrect) {
            // Replay target note.
            oscTarget[simonTargetRow][simonTargetCol] = gameAmps[simonTargetRow];
          } else {
            // Play a fail tone on oscillator (0,0) (adjust frequency if desired).
            oscTarget[0][0] = gameAmps[0];
          }
          
          // After feedback, either advance or end the game.
          if (currentMillis >= simonTimer) {
            if (simonCorrect) {
              if (simonLevel < 4) {
                simonLevel++;        // Increase difficulty.
                startSimonRound();   // Start the next round.
              } else {
                Serial.println("Simon Says: You won!");
                simonState = SIMON_IDLE;  // Return to normal synth mode.
              }
            } else {
              Serial.println("Simon Says: Incorrect. Game over.");
              simonLevel = 1;     // Reset level.
              simonState = SIMON_IDLE;
            }
          }
        }
        break;
        
      default:
        break;
    }
    
    // In Simon game mode, skip the normal synth joystick/finger updates.
    return;
  }
  
  // --- Normal Synth Mode ---
  // Joystick Frequency Shift:
  int leftY = mozziAnalogRead(JOY_LEFT_Y_PIN);
  int rightY = mozziAnalogRead(JOY_RIGHT_Y_PIN);
  
  // Map deviation from center to a shift percentage (~±6% at full deviation).
  float leftShift = ((leftY - 512) / 512.0) * 0.06;
  float rightShift = ((rightY - 512) / 512.0) * 0.06;
  
  // Update each oscillator's frequency.
  // Use the left joystick’s shift for the top half (rows 0-1)
  // and the right joystick’s shift for the bottom half (rows 2-3).
  for (uint8_t i = 0; i < 4; i++) {
    float multiplier = (i < 2) ? (1.0 + leftShift) : (1.0 + rightShift);
    for (uint8_t j = 0; j < 4; j++) {
      int newFreq = freqTable[i][j] * multiplier;
      oscs[i][j].setFreq(newFreq);
    }
  }
  
  // Finger Input Amplitude Control:
  // For each right-left finger combination, if both corresponding finger pins are pressed then set amplitude.
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
  
  mixedOutput /= 16; // Scale overall output.
  return MonoOutput::from16Bit(mixedOutput);
}


// ----- Main Loop -----
// In addition to calling audioHook(), we print finger press events for debugging.
void loop() {
  audioHook();
  
  // Debug: print finger press events.
  for (uint8_t i = 0; i < 8; i++) {
    bool currentState = (digitalRead(fingerPins[i]) == LOW); // true if pressed
    if (currentState != prevFingerState[i]) {
      prevFingerState[i] = currentState;  // Update stored state.
      if (currentState) {
        Serial.print("Finger ");
        Serial.print(i + 1);
        Serial.println(" pressed");
      }
    }
  }
}

