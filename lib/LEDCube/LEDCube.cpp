#include "LEDCube.h"

LEDCube::LEDCube() {
    writeBuffer = 0;
    displayBuffer = 1;

    clear();

    for (active_z = 0; active_z < CUBE_SIZE_Z; active_z++) {
        pinMode(pins_z[active_z], OUTPUT);
        digitalWrite(pins_z[active_z], LOW);
    }

    pinMode(OUTPUT_ENABLE, OUTPUT);
    digitalWrite(OUTPUT_ENABLE, LOW); // Turn on output (keep it on during cycling or led string capacitance wont discharge quickly enough)

    ledDriver = new PCA9685(PCA9685_ADDRESS);
    Wire.begin(); // We need to call it twice. Otherwise i2c does not work :/
    ledDriver->begin(1000000); // Use FM+ (1Mhz)
    ledDriver->setFrequency(1600); // 1.6khz is maximum PWM Freq for PCA9685
}

void LEDCube::clear() {
    memset(ledMatrix[writeBuffer], 0, sizeof(ledMatrix[writeBuffer]));
}

void LEDCube::swapBuffers() {
    if (writeBuffer == 0) {
        displayBuffer = 0;
        writeBuffer = 1;
    } else {
        displayBuffer = 1;
        writeBuffer = 0;
    }
}

void LEDCube::update(uint32_t delay_us) {
    this->swapBuffers();
    this->clear();
    delayMicroseconds(delay_us);
}

void LEDCube::setVoxel(uint8_t x, uint8_t y, uint8_t z, uint16_t intensity) {
    ledMatrix[writeBuffer][z][y][x] = intensity;
}

void LEDCube::drawLine(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2, uint16_t intensity) {
    // bresenham 3d line algo
    int8_t dx, dy, dz, xs, ys, zs, p1, p2 = 0;
    dx = abs(x2 - x1);
    dy = abs(y2 - y1);
    dz = abs(z2 - z1);
    (x2 > x1) ? xs = 1 : xs = -1;
    (y2 > y1) ? ys = 1 : ys = -1;
    (z2 > z1) ? zs = 1 : zs = -1;

    setVoxel(x1, y1, z1, intensity);
    
    // Driving axis is X-axis
    if (dx >= dy and dx >= dz) {
        p1 = 2 * dy - dx;
        p2 = 2 * dz - dx; 
        while (x1 != x2) {
            x1 += xs;
            if (p1 >= 0) {
                y1 += ys;
                p1 -= 2 * dx;
            }
            if (p2 >= 0) {
                z1 += zs;
                p2 -= 2 * dx;
            }
            p1 += 2 * dy;
            p2 += 2 * dz;
            setVoxel(x1, y1, z1, intensity);
        }
    } else if (dy >= dx and dy >= dz) { // Driving axis is Y-axis
        p1 = 2 * dx - dy;
        p2 = 2 * dz - dy;
        while (y1 != y2) {
            y1 += ys;
            if (p1 >= 0) {
                x1 += xs;
                p1 -= 2 * dy;
            }
            if (p2 >= 0) {
                z1 += zs;
                p2 -= 2 * dy;
            }
            p1 += 2 * dx;
            p2 += 2 * dz;
            setVoxel(x1, y1, z1, intensity);
        }
    } else { // Driving axis is Z-axis
        p1 = 2 * dy - dz;
        p2 = 2 * dx - dz;
        while (z1 != z2) {
            z1 += zs;
            if (p1 >= 0) {
                y1 += ys;
                p1 -= 2 * dz;
            }
            if (p2 >= 0) {
                x1 += xs;
                p2 -= 2 * dz;
            }
            p1 += 2 * dy;
            p2 += 2 * dx;
            setVoxel(x1, y1, z1, intensity);
        }
    }
}

void LEDCube::drawPrism(uint8_t x, uint8_t y, uint8_t z, int8_t dx, int8_t dy, int8_t dz, uint16_t intensity) {
  // top rectangle
  drawLine(x, y, z, x, y+dy, z, intensity);
  drawLine(x, y+dy, z, x+dx, y+dy, z, intensity);
  drawLine(x+dx, y+dy, z, x+dx, y, z, intensity);
  drawLine(x+dx, y, z, x, y, z, intensity);

  // bottom rectangle
  drawLine(x, y, z+dz, x, y+dy, z+dz, intensity);
  drawLine(x, y+dy, z+dz, x+dx, y+dy, z+dz, intensity);
  drawLine(x+dx, y+dy, z+dz, x+dx, y, z+dz, intensity);
  drawLine(x+dx, y, z+dz, x, y, z+dz, intensity);

  // joining verticals
  drawLine(x, y, z, x, y, z+dz, intensity);
  drawLine(x, y+dy, z, x, y+dy, z+dz, intensity);
  drawLine(x+dx, y+dy, z, x+dx, y+dy, z+dz, intensity);
  drawLine(x+dx, y, z, x+dx, y, z+dz, intensity);
}

void LEDCube::drawCube(uint8_t x, uint8_t y, uint8_t z, int8_t size, uint16_t intensity) {
    drawPrism(x, y, z, size-1, size-1, size-1, intensity);
}

void LEDCube::setAll(uint16_t intensity) {
    for(uint8_t x = 0; x < CUBE_SIZE_X; x++) {
        for (uint8_t y = 0; y < CUBE_SIZE_Y; y++) {
            for (uint8_t z = 0; z < CUBE_SIZE_Z; z++) {
                ledMatrix[writeBuffer][x][y][z] = intensity;
            }
        }
    }
}

void LEDCube::setBuffer(uint16_t (&buffer)[CUBE_SIZE_Z][CUBE_SIZE_Y][CUBE_SIZE_X]) {
    memcpy(ledMatrix[writeBuffer], buffer, sizeof(ledMatrix[writeBuffer]));
}

void LEDCube::draw() {
    digitalWrite(pins_z[active_z], LOW);
    active_z++;
    if (active_z > 3) {
        active_z = 0;
    }
    
    this->sendMatrix(ledMatrix[displayBuffer][active_z]);

    digitalWrite(pins_z[active_z], HIGH);
}

void LEDCube::sendMatrix(uint16_t (&values)[CUBE_SIZE_Y][CUBE_SIZE_X]) {
    uint16_t *buffer[16] = CHANNEL_MAPPING;
    ledDriver->setAllChannels(0); // Clear all channels before rewriting to prevent ghosting
    ledDriver->setChannelsIndiviual(buffer);
}