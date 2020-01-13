#include "LEDCube.h"

Voxel::Voxel(LEDCube *cube, uint8_t x, uint8_t y, uint8_t z, uint16_t intensity) {
    this->cube = cube;
    trail = 0;
    this->setIntensity(intensity);
    this->moveTo(x, y, z);
}

void Voxel::moveTo(uint8_t x, uint8_t y, uint8_t z) {
    cur_pos.x = x;
    cur_pos.y = y;
    cur_pos.z = z;
    updateCube();
}

void Voxel::move(int8_t x, int8_t y, int8_t z) {
    cur_pos.x += x;
    cur_pos.y += y;
    cur_pos.z += z;
    updateCube();
}

void Voxel::setTrail(uint8_t i) {
    trail = i;
}

void Voxel::setIntensity(uint16_t intensity) {
    cur_pos.intensity = intensity;
}

uint8_t Voxel::getX() {
    return cur_pos.x;
}
uint8_t Voxel::getY() {
    return cur_pos.y;
}
uint8_t Voxel::getZ() {
    return cur_pos.z;
}

void Voxel::updateCube() {
    this->cube->setVoxel(cur_pos.x, cur_pos.y, cur_pos.z, cur_pos.intensity);
    if (trail > 0) {
        for (uint8_t i = 0; i < trail; i++) {
            if (history.size() < i+1) {
                break;
            }
            //FIXME: Make the intensity degrading more dynamic (log)
            this->cube->setVoxel(history[i].x, history[i].y, history[i].z, history[i].intensity / ((i+1)*3));
        }
        history.unshift(cur_pos);
    }
}