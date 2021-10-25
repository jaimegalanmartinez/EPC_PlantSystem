#include "mbed.h"
#include "TCS3472_I2C.h"
TCS3472_I2C rgb_sensor (PA_10, PA_9);
DigitalOut Red(LED4);
DigitalOut Green(LED1);
DigitalOut Blue(LED3);
InterruptIn button(PB_2);
Ticker periodicMeasurement;


Timer debounce;
Ticker led_ticker;
Timeout state_transition_to;


