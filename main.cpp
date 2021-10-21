/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "TCS3472_I2C/TCS3472_I2C.h"

#define PERIOD_MEASUREMENT_TEST 2000000
#define PERIOD_MEASUREMENT_NORMAL 30000000
//I2C i2c(PA_10, PA_9); //pins for I2C communication (SDA, SCL)
//Serial pc(USBTX, USBRX);
int rgb_readings[4]; // declare a 4 element array to store RGB sensor readings
//int sensor_addr = 41 << 1;
enum Mode{ TEST, NORMAL, ADVANCED};

TCS3472_I2C rgb_sensor (PA_10, PA_9);

DigitalOut TestMode_LED(LED1);
DigitalOut NormalMode_LED(LED2);
DigitalOut AdvancedMode_LED(LED3);
InterruptIn button(PB_2);
Ticker periodicMeasurement;

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
	
		TestMode_LED    = 0;
		NormalMode_LED  = 0;
		AdvancedMode_LED= 0;
		//Set high interrupt
		/*rgb_sensor.enableInterrupt();
		rgb_sensor.setHighInterruptThreshold(20000);
		rgb_sensor.setLowInterruptThreshold(100);*/
    while(1) {
			if(read_sensors_flag){
				read_sensors_flag=false;
				//RGB sensor
				rgb_sensor.getAllColors( rgb_readings ); // read the sensor to get red, green, and blue color data along with overall brightness

				//print values
				printf("Colour: Clear=%d, Red=%d, Green=%d, Blue=%d\n",rgb_readings[0],rgb_readings[1],rgb_readings[2],rgb_readings[3]);
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
