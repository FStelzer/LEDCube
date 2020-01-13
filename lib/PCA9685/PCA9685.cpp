/**
* PCA9685.cpp - Arduino library to control PCA9685 chip.
*/
#include "PCA9685.h"
#include <stdarg.h>
#include <stdio.h>

PCA9685::PCA9685(uint8_t address) {
  _address = address;
}

void PCA9685::begin(uint32_t frequency) {
  Wire.begin();
  Wire.setClock(frequency);
  restart();
}

void PCA9685::restart() {
  // wake
  wake();

  // go to sleep so we can restart
  sleep();

 /** 
  * Restart and set Mode1 register to our prefered mode:
  * Restart         : bit 7 = 1 - Will revert to 0 after restart
  * Internal Clock  : bit 6 = 0
  * Auto Increment  : bit 5 = 1
  * Normal Mode     : bit 4 = 0
  * SUB1 Disabled   : bit 3 = 0
  * SUB2 Disabled   : bit 2 = 0
  * SUB3 Disabled   : bit 1 = 0
  * ALLCALL Enabled : bit 0 = 1
  *
  * B10100001 == 0xA1
  * B11110001 == 
  */
  _write8bits(PCA9685_MODE1, 0xA1);

  // delay at least 500 us to wake 
  delay(1);

 /**
  * Set Mode2 register to our prefered mode:
  * Reserved                        : bits 7 through 5 are reserved
  * Output logic state not inverted : bit 4 = 0
  * Outputs change on STOP command  : bit 3 = 0
  * All outputs are configured with
  * a totem pole structure          : bit 2 = 1
  * When OE = 1 (output drivers not
  * enabled), LEDn = 0              : bit 1 & 0 = 00
  *
  * B00000100 == 0x04
  * B00010010 == 0x12 direct led drive
  * B00110010 == 0x32
  */
  _write8bits(PCA9685_MODE2, 0x12);
}

void PCA9685::wake()
{
  // get current mode1
  uint8_t mode1 = _read8bits(PCA9685_MODE1);

  // set normal mode by setting bit 4 to 0
  bitClear(mode1, 4);

  // wake
  _write8bits(PCA9685_MODE1, mode1);

  // delay at least 500 us 
  delay(1);
}

void PCA9685::sleep()
{
  // get current mode1
  uint8_t mode1 = _read8bits(PCA9685_MODE1);

  // set sleep mode by setting bit 4 to 1
  bitSet(mode1, 4);

  // go to sleep
  _write8bits(PCA9685_MODE1, mode1);

  // delay at least 500 us 
  delay(1);
}

/**
* Set PWM Frequency 40-1000Hz, Default 200Hz
*/
void PCA9685::setFrequency(uint16_t freq)
{
  uint8_t prescale = round(((float)25000000 / (float)(freq * (long)4096))) - 1;
  _setPreScale(prescale);
}

/**
* PCA9685 PWM set frequency prescale
*/
void PCA9685::_setPreScale(uint8_t prescale)
{
  // must be in sleep mode to set the prescaler
  sleep();
  // set the prescaler
  _write8bits(PCA9685_PRESCALE, prescale);
  wake();
}

uint8_t PCA9685::_read8bits(uint8_t addr) {
  Wire.beginTransmission((uint8_t)_address);
  Wire.write((uint8_t)addr);
  Wire.endTransmission();

  Wire.requestFrom((uint8_t)_address, (uint8_t)1);
  return Wire.read();
}

void PCA9685::_write8bits(uint8_t reg, uint8_t value) {
  Wire.beginTransmission((uint8_t)_address);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}

/**
* Get the current 12 bit PWM value
*/
uint16_t PCA9685::getChannel(uint8_t channel)
{
  uint16_t level = 0;
  channel = (channel * 4) + 6;
  Wire.beginTransmission((uint8_t)_address);
  Wire.write((uint8_t)channel);
  Wire.endTransmission();

  Wire.requestFrom((uint8_t)_address, (uint8_t)PCA9685_SUBADR3);
  while (Wire.available()){
    uint8_t onLow = Wire.read();
    uint8_t onHi = Wire.read();
    uint8_t offLow = Wire.read();
    uint8_t offHi = Wire.read();
    level = (word((offHi & 0x0F), offLow) - word((onHi & 0x0F), onLow));
  }
  return level;
}

void PCA9685::setChannelsIndiviual(uint16_t* valuePtrs[PCA9685_CHANNELS]) {
  Wire.beginTransmission(_address);
  Wire.write(PCA9685_LED_BASEADDR);

  //FIXME: Check I2C_TXRX_BUFFER_SIZE to see how many we can fit into a single transaction
  for (uint8_t ch = 0; ch < PCA9685_CHANNELS; ch++){
    if (*valuePtrs[ch] == 0) {
      // Turn Channel fully off
      Wire.write(channelOff, sizeof(channelOff));
    } else if (*valuePtrs[ch] == 4096) {
      // Turn Channel fully on
      Wire.write(channelOn, sizeof(channelOn));
    } else {
      Wire.write(zeroTime, sizeof(zeroTime)); // set all ON time to 0
      Wire.write(lowByte(*valuePtrs[ch])); // set OFF according to value
      Wire.write(highByte(*valuePtrs[ch]));
    }
  }
  Wire.endTransmission();
}

/**
* Set multiple channels to the same 12bit value
*/
void PCA9685::setAllChannels(uint16_t value) {
  Wire.beginTransmission(_address);
  if (value == 0) {
    // Turn Channels fully off
    Wire.write(allChannelsOff, sizeof(allChannelsOff));
  } else if (value == 4096) {
    // Turn Channels fully on
    Wire.write(allChannelsOn, sizeof(allChannelsOn));
  } else {
    Wire.write(PCA9685_LED_BASEADDR);
    for (uint8_t ch = 0; ch < PCA9685_CHANNELS; ch++) {
      Wire.write(zeroTime, sizeof(zeroTime)); // set all ON time to 0
      Wire.write(lowByte(value)); // set OFF according to value
      Wire.write(highByte(value));
    }
  }
  Wire.endTransmission();
}