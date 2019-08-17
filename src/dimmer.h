#ifndef DIMMER_H
#define DIMMER_H

#include <Arduino.h>

#include <RBDdimmer.h>

extern dimmerLamp lightDimmer;
extern dimmerLamp fanDimmer;

void initDimmers();
void setDimmer(dimmerLamp& dimmer, int intensity);

#endif