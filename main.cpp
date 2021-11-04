/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "hardware.h"
#define PERIOD_MEASUREMENT_TEST 2000000
#define PERIOD_MEASUREMENT_NORMAL 2000000//30000000
#define PERIOD_HALF_HOUR 5000000 //1800000000

#define TEMPERATURE_MAX 28
#define TEMPERATURE_MIN 15
#define HUMIDITY_MAX 60
#define HUMIDITY_MIN 40
#define LIGHT_MAX 100
#define LIGHT_MIN 20
#define MOISTURE_MAX 60
#define MOISTURE_MIN 20

/*
	Normal ranges:
temperature: 18-28ºC
humidity: 40-60%
Ambinet light: 20-100%
Soil humidity: 40-60%
Acceleration: Z axis > X axis && Z axis > Y axis

	Color codes when out of range:
temperature: LED off
humidity: White (RED + GREEN + BLUE)
Ambient light: Margenta (RED + BLUE)
Soil humidity: Cian (GREEN + BLUE)
Color: yellow (RED + GREEN)
Accelerometer: red (RED)
*/

//Struct that stores all previous values in 1 hour to be processed later.
struct Log{
	float temperature_max;
	float temperature_min;
	float temperature_avg;
	float temperature_sum;
	int temperature_num_samples;
	
	float humidity_max;
	float humidity_min;
	float humidity_avg;
	float humidity_sum;
	int humidity_num_samples;
	
	float light_max;
	float light_min;
	float light_avg;
	float light_sum;
	int light_num_samples;
	
	float moisture_max;
	float moisture_min;
	float moisture_avg;
	float moisture_sum;
	int moisture_num_samples;
	
	int color_red;
	int color_blue;
	int color_green;
	
	float accel_x_max;
	float accel_x_min;
	float accel_y_max;
	float accel_y_min;
	float accel_z_max;
	float accel_z_min;
	
};
Log log_values;//Global instance of the struct

int rgb_readings[4]; // declare a 4 element array to store RGB sensor readings

enum Mode{TEST,NORMAL,ADVANCED};

char *dominant_color;
bool read_sensors_flag;
bool user_button_flag;
bool half_hour_flag;
bool full_hour_flag;

void half_hour_irq()
{
		half_hour_flag=true;
}
void read_sensors()
{
		read_sensors_flag=true;
}
void user_button()
{
		user_button_flag=true;
}
/*
Algorithm to set the dominant_color depending on the rgb_readings from the sensor.
The color update is done after the reading of all sensors.
*/
void set_dominant_color(){
	if(rgb_readings[1]>rgb_readings[2] && rgb_readings[1]>=rgb_readings[3]){//If max=RED
		dominant_color="RED";
	}else if(rgb_readings[2]>rgb_readings[1] && rgb_readings[2]>rgb_readings[3]){//If max=Green
		dominant_color="GREEN";
	}else if(rgb_readings[3]>rgb_readings[1] && rgb_readings[3]>rgb_readings[2]){//If max=Blue
		dominant_color="BLUE";					
	}else{//If 2 or 3 values are the same, then it is considered as RED
		dominant_color="RED";
	}
}
/*
Initialize the log_values variable 
*/
void initLog(){
	log_values.temperature_max=-60;
	log_values.temperature_min=200;
	log_values.temperature_avg=0;
	log_values.temperature_sum=0;
	log_values.temperature_num_samples=0;
	
	log_values.humidity_max=0;
	log_values.humidity_min=100;
	log_values.humidity_avg=0;
	log_values.humidity_sum=0;
	log_values.humidity_num_samples=0;
	
	log_values.light_max=0;
	log_values.light_min=100;
	log_values.light_avg=0;
	log_values.light_sum=0;
	log_values.light_num_samples=0;
	
	log_values.moisture_max=0;
	log_values.moisture_min=100;
	log_values.moisture_avg=0;
	log_values.moisture_sum=0;
	log_values.moisture_num_samples=0;
	
	log_values.color_red=0;
	log_values.color_blue=0;
	log_values.color_green=0;
	
  log_values.accel_x_max=-99;
	log_values.accel_x_min=99;
	log_values.accel_y_max=-99;
	log_values.accel_y_min=99;
	log_values.accel_z_max=-99;
	log_values.accel_z_min=99;
}
/*
Updates the log_values variable with the new values recorded
*/
void updateLog(float temperature, float humidity, float moisture, float light, char* dominant_color, float accel_values[],char *dominantColor){
	if(log_values.temperature_max< temperature)
		log_values.temperature_max=temperature;
	if(log_values.temperature_min> temperature)
		log_values.temperature_min=temperature;
	log_values.temperature_sum+=temperature;
	log_values.temperature_num_samples++;
	
	if(log_values.humidity_max< humidity)
		log_values.humidity_max=humidity;
	if(log_values.humidity_min> humidity)
		log_values.humidity_min=humidity;
	log_values.humidity_sum+=humidity;
	log_values.humidity_num_samples++;
	
	if(log_values.moisture_max< moisture)
		log_values.moisture_max=moisture;
	if(log_values.moisture_min> moisture)
		log_values.moisture_min=moisture;
	log_values.moisture_sum+=moisture;
	log_values.moisture_num_samples++;
	
	if(log_values.light_max< light)
		log_values.light_max=light;
	if(log_values.light_min> light)
		log_values.light_min=light;
	log_values.light_sum+=light;
	log_values.light_num_samples++;
	
	if(dominantColor=="RED")
		log_values.color_red++;
	else if(dominantColor=="BLUE")
		log_values.color_blue++;
	else if(dominantColor=="GREEN")
		log_values.color_green++;
	
	if(log_values.accel_x_max< accel_values[0])
		log_values.accel_x_max=accel_values[0];
	if(log_values.accel_x_min> accel_values[0])
		log_values.accel_x_min=accel_values[0];
	if(log_values.accel_y_max< accel_values[1])
		log_values.accel_y_max=accel_values[1];
	if(log_values.accel_y_min> accel_values[1])
		log_values.accel_y_min=accel_values[1];
	if(log_values.accel_z_max< accel_values[2])
		log_values.accel_z_max=accel_values[2];
	if(log_values.accel_z_min> accel_values[2])
		log_values.accel_z_min=accel_values[2];
	
}
void checkRange(int32_t temperature,uint32_t humidity,float light_value_f,float moisture_value_f,float accel_values [],char *dominantColor){
	if(temperature>TEMPERATURE_MAX || temperature<TEMPERATURE_MIN)
		RGB_LED=0b000;
	else	if(humidity>HUMIDITY_MAX || humidity<HUMIDITY_MIN){
		RGB_LED=0b111;
	}else if(light_value_f>LIGHT_MAX || light_value_f<LIGHT_MIN){
		RGB_LED=0b101;
	}else if(moisture_value_f>MOISTURE_MAX || moisture_value_f<MOISTURE_MIN){
		RGB_LED=0b110;
	}else if(dominantColor!="GREEN"){
		RGB_LED=0b011;
	}else if( !(accel_values[2]>accel_values[1]) || !(accel_values[2] > accel_values[0])){
		RGB_LED=0b001;
	}else if(dominantColor=="GREEN"){//If no errors=GREEN
		RGB_LED=0b010;
	}
}
Mode mode = NORMAL;// TEST;
int main() {
		//Initializarion
		initLog();
		RGB_LED=0b000;
		//Turn on color sensor
			//Get ENABLE register		
		rgb_sensor.enablePowerAndRGBC();
	  rgb_sensor.enableWait();
		rgb_sensor.setWaitTime(1500);
		
		GPS_sensor.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //these commands are defined in MBed_Adafruit_GPS.h; a link is provided there for command creation
    GPS_sensor.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    GPS_sensor.sendCommand(PGCMD_ANTENNA);
	  halfHourTicker.attach_us(&half_hour_irq,PERIOD_HALF_HOUR);
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
				float temperature_f =tempHumSensor.get_temperature()/1000.0;//Value is multiplied by 1000
				float humidity_f = tempHumSensor.get_humidity()/1000.0;//Value is multiplied by 1000
				//GPS
				//Shift UTC hour to Madrid
				GPS_sensor.hour++;
				if(GPS_sensor.hour>24){
					GPS_sensor.hour=0;
				}
				//Update RGB LED with color depending on state and if there are out of range values
				
				//If in state Normal
				if(mode == NORMAL){
					//Check errors
					checkRange(temperature_f,humidity_f,light_value_f,moisture_value_f,accel_values,dominant_color );
				}else if(mode == TEST){//If in state TEST
					if(dominant_color=="RED"){//If max=RED
						RGB_LED=0b001;
					}else if(dominant_color=="GREEN"){//If max=Green
						RGB_LED=0b010;
					}else if(dominant_color=="BLUE"){//If max=Blue
						RGB_LED=0b100;	
					}
				}
				//Update log_values with the new recorded values
				updateLog(temperature_f,humidity_f,moisture_value_f,light_value_f,dominant_color,accel_values,dominant_color);
				//print values
				printf("GPS: #Sats: %d, Lat(UTC): %f, Long(UTC): %f, Altitude: %.0fm, GPS_time: %d:%d:%d\n",GPS_sensor.satellites,GPS_sensor.latitude/100,
				GPS_sensor.longitude/100,GPS_sensor.altitude,GPS_sensor.hour,GPS_sensor.minute,GPS_sensor.seconds);
				printf("TEMP/HUM: Temperature: %.1f C, Relative Humidity: %.1f%%\n",temperature_f,humidity_f);
				printf("ACCELEROMETER: X_axis=%.2fg, Y_axis=%.2fg, Z_axis=%.2fg\n",accel_values[0],accel_values[1],accel_values[2]);
				printf("LIGHT: %.1f%%\n",light_value_f);
				printf("SOIL MOISTURE: %.1f%%\n",moisture_value_f);
				printf("COLOR SENSOR: Clear=%d, Red=%d, Green=%d, Blue=%d   -- Dominant color: %s\n\n",rgb_readings[0],rgb_readings[1],rgb_readings[2],rgb_readings[3],dominant_color);
			}
			//Change mode on button click
			if(user_button_flag){
				user_button_flag=false;
				if(mode==TEST) mode=NORMAL;
				else if (mode==NORMAL) mode = ADVANCED;
				else if (mode==ADVANCED) mode = TEST;
				//Update tickers and LED states
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
					if(half_hour_flag){
						half_hour_flag=false;
						if(full_hour_flag){
							full_hour_flag=false;
							//Calculate average
							log_values.temperature_avg=log_values.temperature_sum/log_values.temperature_num_samples;
							log_values.humidity_avg=log_values.humidity_sum/log_values.humidity_num_samples;
							log_values.light_avg=log_values.light_sum/log_values.light_num_samples;
							log_values.moisture_avg=log_values.moisture_sum/log_values.moisture_num_samples;
							
							char* dominant_color_final;
							if(log_values.color_red>log_values.color_green && log_values.color_red> log_values.color_blue)
								dominant_color_final="RED";
							else if(log_values.color_green>log_values.color_blue && log_values.color_green> log_values.color_red) 
								dominant_color_final="GREEN";
							else if(log_values.color_blue>log_values.color_red && log_values.color_blue > log_values.color_green)
								dominant_color_final="BLUE";
							printf("\n\n\n------------SUMARY VALUES 1 HOUR---------\n");
							printf("TEMP: Max: %.1f C, Min: %.1f C, Avg %.1f C\n",log_values.temperature_max,log_values.temperature_min,log_values.temperature_avg);
							printf("HUM: Max: %.1f%%, Min: %.1f%%, Avg %.1f%%\n",log_values.humidity_max,log_values.humidity_min,log_values.humidity_avg);
							printf("LIGHT: Max: %.1f%%, Min: %.1f%%, Avg %.1f%%\n",log_values.light_max,log_values.light_min,log_values.light_avg);
							printf("MOISTURE: Max: %.1f%%, Min: %.1f%%, Avg %.1f%%\n",log_values.moisture_max,log_values.moisture_min,log_values.moisture_avg);
							printf("COLOR: Dominant color: %s\n",dominant_color_final);
							printf("ACCELEROMETER: X: Max: %.1f, Min: %.1f. Y: Max: %.1f, Min: %.1f. Z: Max: %.1f, Min: %.1f.\n",log_values.accel_x_max,log_values.accel_x_min,log_values.accel_y_max,log_values.accel_y_min,log_values.accel_z_max,log_values.accel_z_min);
							printf("---------------------\n\n\n");
							initLog();
						}else
								full_hour_flag=true;
					}
				break;
				case ADVANCED:
					
					break;
			}
    }
}
