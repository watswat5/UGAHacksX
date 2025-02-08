#include <Mozzi.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>

#define RIGHT_INDEX 19
#define RIGHT_MIDDLE 21
#define RIGHT_RING 32
#define LEFT_INDEX 22
#define LEFT_MIDDLE 23
#define LEFT_RING 12

Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin1(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin2(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin3(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin4(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin5(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin6(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin7(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin8(SIN2048_DATA);

uint8_t sin1_v = 0, sin2_v = 0, sin3_v = 0, sin4_v = 0;
uint8_t sin5_v = 0, sin6_v = 0, sin7_v = 0, sin8_v = 0;

void setup() {
  startMozzi();

  pinMode(RIGHT_INDEX, INPUT_PULLUP);
  pinMode(RIGHT_MIDDLE, INPUT_PULLUP);
  pinMode(RIGHT_RING, INPUT_PULLUP);
  pinMode(LEFT_INDEX, INPUT_PULLUP);
  pinMode(LEFT_MIDDLE, INPUT_PULLUP);
  pinMode(LEFT_RING, INPUT_PULLUP);

  sin1.setFreq(262);  // C4
  sin2.setFreq(330);  // E4
  sin3.setFreq(392);  // G4
  sin4.setFreq(494);  // B4
  sin5.setFreq(523);  // C5
  sin6.setFreq(659);  // E5
  sin7.setFreq(784);  // G5
  sin8.setFreq(988);  // B5

  Serial.begin(115200);
}

void updateControl() {
  sin1_v = (digitalRead(RIGHT_INDEX) == LOW && (digitalRead(LEFT_INDEX) == LOW || digitalRead(LEFT_MIDDLE) == LOW || digitalRead(LEFT_RING) == LOW)) ? 255 : 0;
  sin2_v = (digitalRead(RIGHT_MIDDLE) == LOW && (digitalRead(LEFT_INDEX) == LOW || digitalRead(LEFT_MIDDLE) == LOW || digitalRead(LEFT_RING) == LOW)) ? 255 : 0;
  sin3_v = (digitalRead(LEFT_INDEX) == LOW && (digitalRead(RIGHT_INDEX) == LOW || digitalRead(RIGHT_MIDDLE) == LOW || digitalRead(RIGHT_RING) == LOW)) ? 255 : 0;
  sin4_v = (digitalRead(LEFT_MIDDLE) == LOW && (digitalRead(RIGHT_INDEX) == LOW || digitalRead(RIGHT_MIDDLE) == LOW || digitalRead(RIGHT_RING) == LOW)) ? 255 : 0;
  sin5_v = (digitalRead(RIGHT_RING) == LOW && (digitalRead(LEFT_INDEX) == LOW || digitalRead(LEFT_MIDDLE) == LOW || digitalRead(LEFT_RING) == LOW)) ? 255 : 0;
  sin6_v = (digitalRead(RIGHT_RING) == LOW && (digitalRead(LEFT_INDEX) == LOW || digitalRead(LEFT_MIDDLE) == LOW || digitalRead(LEFT_RING) == LOW)) ? 255 : 0;
  sin7_v = (digitalRead(LEFT_RING) == LOW && (digitalRead(RIGHT_INDEX) == LOW || digitalRead(RIGHT_MIDDLE) == LOW || digitalRead(RIGHT_RING) == LOW)) ? 255 : 0;
  sin8_v = (digitalRead(LEFT_RING) == LOW && (digitalRead(RIGHT_INDEX) == LOW || digitalRead(RIGHT_MIDDLE) == LOW || digitalRead(RIGHT_RING) == LOW)) ? 255 : 0;

  Serial.println(sin1_v);
  Serial.println(sin2_v);
  Serial.println(sin3_v);
  Serial.println(sin4_v);
  Serial.println(sin5_v);
  Serial.println(sin6_v);
  Serial.println(sin7_v);
  Serial.println(sin8_v);
}

AudioOutput updateAudio() {
  int16_t mixedOutput = ((sin1.next() * sin1_v) / 8) +
                        ((sin2.next() * sin2_v) / 8) +
                        ((sin3.next() * sin3_v) / 8) +
                        ((sin4.next() * sin4_v) / 8) +
                        ((sin5.next() * sin5_v) / 8) +
                        ((sin6.next() * sin6_v) / 8) +
                        ((sin7.next() * sin7_v) / 8) +
                        ((sin8.next() * sin8_v) / 8);
  return MonoOutput::from16Bit(mixedOutput);
}

void loop() {
  audioHook();
}



