#ifndef MBED_BEEP_H
#define MBED_BEEP_H
 
#include "mbed.h"
 
 
namespace mbed {

class Beep {
 
public:

    Beep (PinName pin);

    void beep (float frequency, float time);

    void nobeep();

    void tone(float freq, float time);
 
private :
    PwmOut _pwm;
    Timeout toff;
};
 
}
#endif
 
            
