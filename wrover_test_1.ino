#include <FixMath.h>
#include <Mozzi.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>

#define THUMB_BUTTON_RIGHT
#define THUMB_BUTTON_LEFT

#define RIGHT_INDEX 19
#define RIGHT_MIDDLE 21
#define RIGHT_RING
#define RIGHT_PINKEY

#define LEFT_INDEX 22
#define LEFT_MIDDLE 23
#define LEFT_RING
#define LEFT_PINKEY


//speaker outputs to 25, 26
//set oscillators, set volume for each
//sound should not play until both buttons are pressed
//pullup; meaning if button is set to LOW it is being pressed

Oscil <SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> aSin(SIN2048_DATA);

void setup() {
  // put your setup code here, to run once:
  startMozzi();
  /*
  pinMode(THUMB_BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(THUMB_BUTTON_LEFT, INPUT_PULLUP);

  pinMode(RIGHT_INDEX, INPUT_PULLUP);
  pinMode(RIGHT_MIDDLE, INPUT_PULLUP);
  pinMode(RIGHT_RING, INPUT_PULLUP);
  pinMode(RIGHT_PINKEY, INPUT_PULLUP);

  pinMode(LEFT_INDEX, INPUT_PULLUP);
  pinMode(LEFT_MIDDLE, INPUT_PULLUP);
  pinMode(LEFT_RING, INPUT_PULLUP);
  pinMode(LEFT_PINKEY, INPUT_PULLUP);
  */
  //aSin.setFreq(440);
  pinMode(19, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
  pinMode(22, INPUT_PULLUP);
  pinMode(23, INPUT_PULLUP);
}

void updateControl(){
  if(digitalRead(RIGHT_INDEX) == LOW){
    aSin.setFreq(200);
  }
  
  if(digitalRead(RIGHT_MIDDLE) == LOW){
    aSin.setFreq(250);
  }
  
  if(digitalRead(LEFT_INDEX) == LOW){
    aSin.setFreq(350);
  }
  
  if(digitalRead(LEFT_MIDDLE) == LOW){
    aSin.setFreq(400);
  }
}

AudioOutput_t updateAudio(){
  MonoOutput::from16Bit(aSin.next());
}

void loop() {
  // put your main code here, to run repeatedly:
  audioHook();
}
