#include "mbed.h"
#include "TCS3472_I2C/TCS3472_I2C.h"
#include "Accel_MMA8451Q/MMA8451Q.h"
#include "Si7021/Si7021.h"

MMA8451Q accel_sensor(PB_9, PB_8,0x1d<<1);
TCS3472_I2C rgb_sensor (PB_9, PB_8);
Si7021 tempHumSensor(PB_9, PB_8);

DigitalOut TestMode_LED(LED1);
DigitalOut NormalMode_LED(LED2);
DigitalOut AdvancedMode_LED(LED3);
InterruptIn button(PB_2);
AnalogIn moistureSensor(PA_0);
AnalogIn lightSensor(PA_4);
BusOut RGB_LED(PH_0,PH_1,PB_13);

Ticker periodicMeasurement;

