#include <Arduino.h>
#undef I2C_TXRX_BUFFER_SIZE
#define I2C_TXRX_BUFFER_SIZE 64
#include "HardwareTimer.h"
#include "animations.h"
#include "LEDCube.h"

#define BTN_1 PA1

HardwareTimer MosTimer(TIM1);

LEDCube *Cube;

void mosTimerInterrupt(HardwareTimer*) {
    Cube->draw();
}

void buttonPressed() {  
  if (animation_mode >= animationListCount) {
    animation_mode_new = 0;
  } else {
    animation_mode_new = animation_mode + 1;
  }
}

void setup() {
    Serial.begin(115200);

    randomSeed(analogRead(MIC_IN));

    pinMode(BTN_1, INPUT);
    attachInterrupt(digitalPinToInterrupt(BTN_1), buttonPressed, FALLING);

    analogReadResolution(ADC_RESOLUTION);

    Cube = new LEDCube();

    MosTimer.setMode(1, TIMER_OUTPUT_COMPARE);
    // Writing all 16 I2C PWM Values takes 920us in FM+ + 95us for clearing them before
    // 150hz per LED/ROW with 4 layers is flicker free and still quite bright
    MosTimer.setOverflow(600, HERTZ_FORMAT);
    MosTimer.attachInterrupt(mosTimerInterrupt);
    MosTimer.resume();
}

void loop() {
    bool init = false;
    if (animation_mode_new != animation_mode) {
      animation_mode = animation_mode_new;
      init = true;
      Cube->clear();
    }

    animationList[animation_mode](init);
    
    /*Serial.print("VRef(mv)= ");
    int32_t VRef = readVref();
    Serial.print(VRef);

    Serial.print("\tTemp(°C)= ");
    Serial.print(readTempSensor(VRef));

    Serial.print("\tA0(mv)= ");
    Serial.println(readVoltage(VRef, A0));*/
}