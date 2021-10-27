/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "hardware.h"

#define PERIOD_MEASUREMENT_TEST 2000000
#define PERIOD_MEASUREMENT_NORMAL 30000000


int rgb_readings[4]; // declare a 4 element array to store RGB sensor readings

enum Mode{TEST,NORMAL,ADVANCED};

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

void set_dominant_color(){
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
}

Mode mode = TEST;
int main() {
		//Initializarion
		//Turn on color sensor
			//Get ENABLE register		
		rgb_sensor.enablePowerAndRGBC();
	  rgb_sensor.enableWait();
		rgb_sensor.setWaitTime(1500);
		
		GPS_sensor.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //these commands are defined in MBed_Adafruit_GPS.h; a link is provided there for command creation
    GPS_sensor.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    GPS_sensor.sendCommand(PGCMD_ANTENNA);
	    
		periodicMeasurement.attach_us(&read_sensors,PERIOD_MEASUREMENT_TEST);
		button.mode(PullUp);
		button.fall(&user_button);
	
		if(!tempHumSensor.check()){
			printf("Temperature and humidity sensor error");
		}
	
		TestMode_LED     = 0;
		NormalMode_LED   = 0;
		AdvancedMode_LED = 0;
		//Set high interrupt
		/*rgb_sensor.enableInterrupt();
		rgb_sensor.setHighInterruptThreshold(20000);
		rgb_sensor.setLowInterruptThreshold(100);*/
    while(1) {
			//GPS
				char c = GPS_sensor.read();
				//printf("%s",&c);
				//If a NMEA message is received
				if (GPS_sensor.newNMEAreceived()) {
					if (!GPS_sensor.parse(GPS_sensor.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
				 // we can fail to parse a sentence in which case we should just wait for another
					//printf("Fail to parse\n");
					;
				}
			if(read_sensors_flag){
				read_sensors_flag=false;
				
				//RGB sensor
				rgb_sensor.getAllColors( rgb_readings ); // read the sensor to get red, green, and blue color data along with overall brightness
				//Dominant color
				set_dominant_color();
				//Moisture sensor
				uint16_t moisture_value = moistureSensor.read_u16();
				float moisture_value_f= moisture_value*100.0/65536.0;
				//Light sensor
				uint16_t light_value = lightSensor.read_u16();
				float light_value_f= light_value*100.0/65536.0;
				//Acceleration sensor
				float accel_values [3];
				accel_sensor.getAccAllAxis(accel_values);
				//Temperature and humidity
				tempHumSensor.get_data();
				int32_t temperature =tempHumSensor.get_temperature();//Value is multiplied by 1000
				uint32_t humidity = tempHumSensor.get_humidity();//Value is multiplied by 1000
				
				//GPS
				
				//print values
				printf("GPS: #Sats: %d, Lat(UTC): %f, Long(UTC): %f, Altitude: %.0fm, GPS_time: %d:%d:%d\n",GPS_sensor.satellites,GPS_sensor.latitude/100,
				GPS_sensor.longitude/100,GPS_sensor.altitude,GPS_sensor.hour,GPS_sensor.minute,GPS_sensor.seconds);
				printf("TEMP/HUM: Temperature: %.1f C, Relative Humidity: %.1f%%\n",temperature/1000.0,humidity/1000.0);
				printf("ACCELEROMETER: X_axis=%.2fg, Y_axis=%.2fg, Z_axis=%.2fg\n",accel_values[0],accel_values[1],accel_values[2]);
				printf("LIGHT: %.1f%%\n",light_value_f);
				printf("SOIL MOISTURE: %.1f%%\n",moisture_value_f);
				printf("COLOR SENSOR: Clear=%d, Red=%d, Green=%d, Blue=%d   -- Dominant color: %s\n\n",rgb_readings[0],rgb_readings[1],rgb_readings[2],rgb_readings[3],dominant_color);
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