/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "TCS3472_I2C.h"
#include "mbed.h"
#include "MMA8451Q.h"

#define PERIOD_MEASUREMENT_TEST 2000000
#define PERIOD_MEASUREMENT_NORMAL 30000000
#define MMA8451Q_SENSOR_ADDR	0x1d

int rgb_readings[4]; // declare a 4 element array to store RGB sensor readings

enum Mode{TEST,NORMAL,ADVANCED};

TCS3472_I2C rgb_sensor (PB_9, PB_8);

DigitalOut TestMode_LED(LED1);
DigitalOut NormalMode_LED(LED2);
DigitalOut AdvancedMode_LED(LED3);
InterruptIn button(PB_2);
AnalogIn moistureSensor(PA_0);
AnalogIn lightSensor(PA_4);
BusOut RGB_LED(PH_0,PH_1,PB_13);
MMA8451Q accel_sensor(PB_9, PB_8,0x1d<<1);

Ticker periodicMeasurement;
char *dominant_color;
bool read_sensors_flag;
bool user_button_flag;
void read_sensors()
{
		read_sensors_flag=true;
}
void user_button()
{
		user_button_flag=true;
}

Mode mode = TEST;
int main() {
		//Initializarion
		//Turn on color sensor
			//Get ENABLE register		
		rgb_sensor.enablePowerAndRGBC();
	  rgb_sensor.enableWait();
		rgb_sensor.setWaitTime(1500);
	
		periodicMeasurement.attach_us(&read_sensors,PERIOD_MEASUREMENT_TEST);
		button.mode(PullUp);
		button.fall(&user_button);
	
		TestMode_LED     = 0;
		NormalMode_LED   = 0;
		AdvancedMode_LED = 0;
		//Set high interrupt
		/*rgb_sensor.enableInterrupt();
		rgb_sensor.setHighInterruptThreshold(20000);
		rgb_sensor.setLowInterruptThreshold(100);*/
    while(1) {
			if(read_sensors_flag){
				read_sensors_flag=false;
				//RGB sensor
				rgb_sensor.getAllColors( rgb_readings ); // read the sensor to get red, green, and blue color data along with overall brightness
				//Dominant color
				if(rgb_readings[1]>rgb_readings[2] && rgb_readings[1]>=rgb_readings[3]){//If max=RED
					RGB_LED=0b001;
					dominant_color="RED";
				}else if(rgb_readings[2]>rgb_readings[1] && rgb_readings[2]>rgb_readings[3]){//If max=Green
					RGB_LED=0b010;
					dominant_color="GREEN";
				}else if(rgb_readings[3]>rgb_readings[1] && rgb_readings[3]>rgb_readings[2]){//If max=Blue
					RGB_LED=0b100;
					dominant_color="BLUE";					
				}else{
					RGB_LED=0b000;
					dominant_color="NONE";
				}
				//Moisture sensor
				uint16_t moisture_value = moistureSensor.read_u16();
				float moisture_value_f= moisture_value*100.0/65536.0;
				//Light sensor
				uint16_t light_value = lightSensor.read_u16();
				float light_value_f= light_value*100.0/65536.0;
				//Acceleration sensor
				float *accel_values= new float[3];
				accel_sensor.getAccAllAxis(accel_values);
				
				//print values
				printf("Accelerometer: x_axis=%.2f, y_axis=%.2f, z_axis=%.2f\n",accel_values[0],accel_values[1],accel_values[2]);
				printf("Light: %.1f%%\n",light_value_f);
				printf("Moisture: %.1f%%\n",moisture_value_f);
				printf("Color: Clear=%d, Red=%d, Green=%d, Blue=%d\n",rgb_readings[0],rgb_readings[1],rgb_readings[2],rgb_readings[3]);
				printf("Dominant color: %s\n\n",dominant_color);
			}
			//Change mode
			if(user_button_flag){
				user_button_flag=false;
				if(mode==TEST) mode=NORMAL;
				else if (mode==NORMAL) mode = ADVANCED;
				else if (mode==ADVANCED) mode = TEST;
				//Update tickers,...
				switch (mode){
					case TEST:
						periodicMeasurement.detach();
						periodicMeasurement.attach_us(&read_sensors,PERIOD_MEASUREMENT_TEST);
						TestMode_LED    = 1;
						NormalMode_LED  =0;
						AdvancedMode_LED   =0;
						break;
					case NORMAL:
						periodicMeasurement.detach();
						periodicMeasurement.attach_us(&read_sensors,PERIOD_MEASUREMENT_NORMAL);
						TestMode_LED    = 0;
						NormalMode_LED  =1;
						AdvancedMode_LED   =0;
					break;
					case ADVANCED:
						periodicMeasurement.detach();
						TestMode_LED    = 0;
						NormalMode_LED  =0;
						AdvancedMode_LED   =1;
						break;
				}
			}
			//Functions only done in one mode:
			switch (mode){
				case TEST:
					
					break;
				case NORMAL:
					
				break;
				case ADVANCED:
					
					break;
			}
    }
}