#ifndef __LEDCube___
#define __LEDCube___

#include "Arduino.h"
#include "PCA9685.h"

#define CUBE_SIZE_X 4
#define CUBE_SIZE_Y 4
#define CUBE_SIZE_Z 4

#define OUTPUT_ENABLE PB3
#define PINS_Z {PB15, PB14, PB13, PB12}

#define PCA9685_ADDRESS 0x40 

// This mapping allows us to have the voxel ordered in memory allowing for easy operations
// but still have the rows/columns connected to the output channels at random
/*#define CHANNEL_MAPPING {&values[0][0], &values[0][1], &values[0][2], &values[0][3], \
                         &values[1][0], &values[1][1], &values[1][2], &values[1][3], \
                         &values[2][0], &values[2][1], &values[2][2], &values[2][3], \
                         &values[3][0], &values[3][1], &values[3][2], &values[3][3]}*/

/*
    LEDCube Board Channel -> LED mapping
    11, 10, 9, 8, 7, 6, 5, 4, 13, 12, 15, 14, 1, 0, 3, 2
*/
#define CHANNEL_MAPPING {&values[3][1], &values[3][0], &values[3][3], &values[3][2], \
                         &values[1][3], &values[1][2], &values[1][1], &values[1][0], \
                         &values[0][3], &values[0][2], &values[0][1], &values[0][0], \
                         &values[2][1], &values[2][0], &values[2][3], &values[2][2]}

class LEDCube{
public:
    LEDCube();
    void draw();
    void setVoxel(uint8_t x, uint8_t y, uint8_t z, uint16_t intensity);
    void drawLine(uint8_t x, uint8_t y, uint8_t z, uint8_t x2, uint8_t y2, uint8_t z2, uint16_t intensity);
    void drawPrism(uint8_t x, uint8_t y, uint8_t z, int8_t dx, int8_t dy, int8_t dz, uint16_t intensity);
    void drawCube(uint8_t x, uint8_t y, uint8_t z, int8_t size, uint16_t intensity);
    void setAll(uint16_t intensity);
    void setBuffer(uint16_t (&buffer)[CUBE_SIZE_Z][CUBE_SIZE_Y][CUBE_SIZE_X]);
    void update(uint32_t delay_us);
    void clear();
private:
    const uint8_t pins_z[CUBE_SIZE_Z] = PINS_Z;
    uint16_t ledMatrix[2][CUBE_SIZE_Z][CUBE_SIZE_Y][CUBE_SIZE_X] = {0};
    uint8_t writeBuffer, displayBuffer;
    volatile uint8_t active_z = 0;

    PCA9685 *ledDriver;

    void swapBuffers();
    void sendMatrix(uint16_t (&values)[CUBE_SIZE_Y][CUBE_SIZE_X]);
};

#include <CircularBuffer.h>
#define POSITION_HISTORY 5

class Voxel{
public:
    Voxel(LEDCube *cube, uint8_t x, uint8_t y, uint8_t z, uint16_t intensity);
    void moveTo(uint8_t x, uint8_t y, uint8_t z);
    void move(int8_t x, int8_t y, int8_t z);
    void setTrail(uint8_t i);
    void setIntensity(uint16_t intensity);
    void updateCube();
    uint8_t getX();
    uint8_t getY();
    uint8_t getZ();
private:
    LEDCube *cube;
    struct Position{
        uint8_t x;
        uint8_t y;
        uint8_t z;
        uint16_t intensity;
    };
    Position cur_pos;
    CircularBuffer<Position, POSITION_HISTORY> history;
    uint8_t trail;
};

#endif