#include <Mozzi.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>

#define RIGHT_INDEX 19
#define RIGHT_MIDDLE 21
#define LEFT_INDEX 22
#define LEFT_MIDDLE 23

Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin1(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin2(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin3(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sin4(SIN2048_DATA);

uint8_t sin1_v = 0, sin2_v = 0, sin3_v = 0, sin4_v = 0;

void setup() {
  startMozzi();

  pinMode(RIGHT_INDEX, INPUT_PULLUP);
  pinMode(RIGHT_MIDDLE, INPUT_PULLUP);
  pinMode(LEFT_INDEX, INPUT_PULLUP);
  pinMode(LEFT_MIDDLE, INPUT_PULLUP);

  sin1.setFreq(262);
  sin2.setFreq(330);
  sin3.setFreq(392);
  sin4.setFreq(494);
  Serial.begin(115200);
}

void updateControl() {
  sin1_v = digitalRead(RIGHT_INDEX) == LOW ? 255 : 0;
  sin2_v = digitalRead(RIGHT_MIDDLE) == LOW ? 255 : 0;
  sin3_v = digitalRead(LEFT_INDEX) == LOW ? 255 : 0;
  sin4_v = digitalRead(LEFT_MIDDLE) == LOW ? 255 : 0;

  Serial.println(sin1_v);
  Serial.println(sin2_v);
  Serial.println(sin3_v);
  Serial.println(sin4_v);
}

AudioOutput updateAudio() {
  int16_t mixedOutput = ((sin1.next() * sin1_v) / 4) +
                        ((sin2.next() * sin2_v) / 4) +
                        ((sin3.next() * sin3_v) / 4) +
                        ((sin4.next() * sin4_v) / 4);
  return MonoOutput::from16Bit(mixedOutput);
}

void loop() {
  audioHook();
}
