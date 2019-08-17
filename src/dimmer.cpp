#include <dimmer.h>

#define outputPinLight 26
#define outputPinFan 27
#define zerocross 25

dimmerLamp lightDimmer(outputPinLight, zerocross);
dimmerLamp fanDimmer(outputPinFan, zerocross);

void initDimmers() {
    lightDimmer.begin(NORMAL_MODE, OFF);
    fanDimmer.begin(NORMAL_MODE, OFF);
}

void setDimmer(dimmerLamp& dimmer, int intensity) {
    intensity =
        std::max(std::min(intensity, 94), 0);  // Gate value between 0 and 94
    if (intensity < 15) {
        dimmer.setMode(NORMAL_MODE);
        dimmer.setState(OFF);
    } else {
        dimmer.setMode(NORMAL_MODE);
        dimmer.setState(ON);
        dimmer.setPower(intensity);
    }
}

