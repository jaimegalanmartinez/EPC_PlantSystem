#include "mbed.h"
#include "TCS3472_I2C/TCS3472_I2C.h"
#include "Accel_MMA8451Q/MMA8451Q.h"
#include "Si7021/Si7021.h"

/*EPC PlantSystem project
* @Authors: Jaime Galan Martinez
*						Victor Aranda Lopez
*/
#ifndef _HARDWARE_H_
#define _HARDWARE_H_

extern MMA8451Q accel_sensor;
extern TCS3472_I2C rgb_sensor;
extern Si7021 tempHumSensor;

extern DigitalOut TestMode_LED, NormalMode_LED, AdvancedMode_LED;
extern InterruptIn button;
extern AnalogIn moistureSensor, lightSensor;
extern BusOut RGB_LED;
extern Ticker periodicMeasurement;

#endif
