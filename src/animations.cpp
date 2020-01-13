#include <Arduino.h>
#include "LEDCube.h"
#include "animations.h"

extern LEDCube *Cube;

void pulseInnerOuter(bool init) {
    static uint16_t i = 0;
    static uint16_t j = 1024;
    static int8_t direction = 1;
    if (init) {
        i = 0; j = 1024; direction = 1;
    }
    if (i >= 1024) { direction = -1; }
    if (i <= 0) { direction = 1; }
    i = i + direction;
    j = j + (direction * -1);

    Cube->setAll(CIEL_10_12(i));

    for(uint8_t a = 1; a <= 2; a++) {
        for (uint8_t b = 1; b <= 2; b++) {
        Cube->setVoxel(a, b, 1, CIEL_10_12(j));
        Cube->setVoxel(a, b, 2, CIEL_10_12(j));
        }
    }

    Cube->update(1000);
} 

void drawCubes(bool init) {
    static uint16_t i = 0;
    static int8_t direction = 1;
    if (init) { i = 0; direction = 1; }
    if ( i > 3 ) { direction = -1; }
    if ( i <= 0 ) { direction = 1; }
    i += direction;
    Cube->drawCube(0, 0, 0, i, CIEL_10_12(i * 256)); Cube->update(100000);
}

void animateBeats(bool init) {
    static uint16_t beat_intensity = 0;
    if (init) { beat_intensity = 0; }
    uint32_t mic_value = (float)analogRead(MIC_IN)-1024.0f;
    float bass_value = bassFilter(mic_value);
    if (bass_value < 0) bass_value = -bass_value;

    //Serial.print(mic_value); Serial.print("  "); Serial.println(bass_value);
    if (bass_value > 35 && beat_intensity >= 0) {
        beat_intensity = 1024;
        Cube->setAll(CIEL_10_12(beat_intensity)); Cube->update(125);
    } else if (beat_intensity > 0) {
        beat_intensity--;
        Cube->setAll(CIEL_10_12(beat_intensity)); Cube->update(50);
    }
}


/****************************
 * 
 * Utitily functions
 * 
 * **************************/

// 20 - 200hz Single Pole Bandpass IIR Filter
float bassFilter(float sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = sample / 9.1f;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0]) + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
    return yv[2];
}

int32_t readVref() {
  return (VREFINT * ADC_RANGE / analogRead(AVREF)); // ADC sample to mV
}

int32_t readTempSensor(int32_t VRef) {
  return (__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(AVG_SLOPE, V25, CALX_TEMP, VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
}

int32_t readVoltage(int32_t VRef, uint32_t pin) {
  return (__LL_ADC_CALC_DATA_TO_VOLTAGE(VRef, analogRead(pin), LL_ADC_RESOLUTION));
}