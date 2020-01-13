#ifndef __ANIMATIONS___
#define __ANIMATIONS___

#include "stm32yyxx_ll_adc.h"

#define MIC_IN PA0

#define CALX_TEMP 25
#define V25       1430
#define AVG_SLOPE 4300
#define VREFINT   1200

#define LL_ADC_RESOLUTION LL_ADC_RESOLUTION_12B
#define ADC_RANGE 4096

// Helper macro to exit loops if animation mode was changed
#define YIELD if (animation_mode_new != animation_mode) return;

// Animation functions
void pulseInnerOuter(bool init);
void drawCubes(bool init);
void animateBeats(bool init);

// List of animations to switch through
static void (*animationList[])(bool) = { pulseInnerOuter, drawCubes, animateBeats };
const static uint8_t animationListCount = (sizeof(animationList)/sizeof(animationList[0])) - 1;
static uint8_t animation_mode = 0;
static volatile uint8_t animation_mode_new = 0;

// Utility functions for animations
float bassFilter(float sample);
int32_t readVref();
int32_t readTempSensor(int32_t VRef);
int32_t readVoltage(int32_t VRef, uint32_t pin);

#endif