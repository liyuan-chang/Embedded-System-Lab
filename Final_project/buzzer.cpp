#include "buzzer.h"
#include "mbed.h"
 
using namespace mbed;


// constructor    
Beep::Beep(PinName pin) : _pwm(pin) {
    _pwm.write(0.0);     // after creating it have to be off
}

// reset
void Beep::nobeep() {
    _pwm.write(0.0);
}

// beep
void Beep::beep(float freq, float time) {
 
    _pwm.period(1.0/freq);
    _pwm.write(1);            // 100% duty cycle - beep on
    rtos::ThisThread::sleep_for(time);    // millisecond
    Beep::nobeep();             // turn off
}

void Beep::tone(float freq, float time) {
    _pwm.period(1.0/freq);
    _pwm.write(0.5);            // 50% duty cycle - beep on
    rtos::ThisThread::sleep_for(time);    // millisecond
    Beep::nobeep();             // turn off
}