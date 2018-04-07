#include "mbed.h"

class RGBLed
{
public:
    RGBLed(PinName redpin, PinName bluepin);
    void write(int color);
private:
    PwmOut _redpin;
    PwmOut _bluepin;
};
 
RGBLed::RGBLed(PinName redpin,  PinName bluepin)
    : _redpin(redpin), _bluepin(bluepin)
{
    _redpin.period(0.0005);
}
 
void RGBLed::write(int color)
{
    float red = ((color && 0xff0000) >> 4) / 255.0;
    float blue = (color && 0x0000ff) / 255.0;
    _redpin = red;
    _bluepin = blue;
}
