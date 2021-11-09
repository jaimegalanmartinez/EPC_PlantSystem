/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "hardware.h"
#include "log_values_sensors.h"
#define PERIOD_MEASUREMENT_TEST 2000000 //2s
#define PERIOD_MEASUREMENT_NORMAL 2000000 //30000000 30s
#define PERIOD_HALF_HOUR 5000000 //1800000000
#define ON  1
#define OFF 0
#define MAIL_QUEUE_SIZE 1
//Event flags used to inform about messages to read 
#define EV_FLAG_READ_SENSORS (1UL << 0) // 00000000000000000000000000000001
#define EV_FLAG_PRINT_INFO (1UL << 9)
#define EV_FLAG_PRINT_INFO_LOGS (1UL << 1)
#define SENSORS_READ_CADENCY_TEST 2000ms
#define SENSORS_READ_CADENCY_NORMAL 5000ms //30s
//Stack size for threads
#define STACK_SIZE_OUTPUT_THREAD 1024
#define STACK_SIZE_MEASURE_THREAD 2048
//Ranges for sensor data
/*#define TEMPERATURE_MAX 28
#define TEMPERATURE_MIN 15
#define HUMIDITY_MAX 60
#define HUMIDITY_MIN 40
#define LIGHT_MAX 100
#define LIGHT_MIN 20
#define MOISTURE_MAX 60
#define MOISTURE_MIN 20
*/
#define TEMPERATURE_MAX 28
#define TEMPERATURE_MIN 15
#define HUMIDITY_MAX 60
#define HUMIDITY_MIN 40
#define LIGHT_MAX 100
#define LIGHT_MIN 20
#define MOISTURE_MAX 42
#define MOISTURE_MIN 20

typedef struct {
		float temperature;
		float humidity;
		float light;
		float moisture;
		float accel_values[3];
	  int rgb_readings[4];
		char dominant_color;
	
} mail_t;

typedef struct {
		Log log_values;
		char dominant_color;

} mail_t_logs;

//Mailbox for communicate the measure thread with the main thread
Mail<mail_t, MAIL_QUEUE_SIZE> sensor_data_mail_box;
//Mailbox for communicate the main thread with the output thread
Mail<mail_t, MAIL_QUEUE_SIZE> print_mail_box;
//Mailbox for communicate sensor_data logs the main thread with the output thread
Mail<mail_t_logs, MAIL_QUEUE_SIZE> print_logs_mail_box;

EventFlags event_flags;
Mutex serial_mutex;
enum Mode{TEST,NORMAL,ADVANCED};
//Threads
Thread measure_thread(osPriorityNormal, STACK_SIZE_MEASURE_THREAD, nullptr, nullptr);//
Thread output_thread(osPriorityNormal,STACK_SIZE_OUTPUT_THREAD,nullptr,nullptr);//Prints the relevant data to the serial port (printf)
/////////////////////////////////////////////

Mode mode = TEST;
volatile bool user_button_flag = false;
bool half_hour_flag = false;

void half_hour_irq();
void user_button();
void checkRange_and_set_RGB_color(float temperature,float humidity,float light_value_f,float moisture_value_f,float accel_values [],char dominantColor);
char set_dominant_color(int rgb_readings[4]);
void set_color_RGB_led(char dominant_color);
void put_sensor_data_on_Mailbox(void);
void put_sensor_data_to_print_mail(mail_t *mail_data_sens);
void put_log_sensor_data_to_print_mail_logs(Log log_hour_values, char dominant_color);
char const* get_str_dominant_color(char dominant_color);
//Thread tasks
void measure_sensors(void);
void GPS_and_print_info_system(void);


int main() {
	Log log_values;//Global instance of the struct
	//Initializarion
	bool full_hour_flag = false;
	initLog(&log_values);
	uint32_t flags_read = 0;
	//RGB_LED OFF
	RGB_LED=0b000;
	//User Button mode and fall interrupt
	button.mode(PullUp);
	button.fall(&user_button);
	//Starting threads
	measure_thread.start(callback(measure_sensors));
	output_thread.start(callback(GPS_and_print_info_system));
	//LEDs off
	TestMode_LED     = OFF;
	NormalMode_LED   = OFF;
	AdvancedMode_LED = OFF;
	//Set high interrupt
	/*rgb_sensor.enableInterrupt();
	rgb_sensor.setHighInterruptThreshold(20000);
	rgb_sensor.setLowInterruptThreshold(100);*/
  while(true) {
		//Change mode
		if(user_button_flag){
			user_button_flag = false;
			if(mode == TEST){
				mode = NORMAL;
				TestMode_LED   = OFF;
				NormalMode_LED = ON;
				halfHourTicker.attach_us(&half_hour_irq,PERIOD_HALF_HOUR);
				
			}else if (mode == NORMAL){ 
				mode = ADVANCED;
				NormalMode_LED   = OFF;
				AdvancedMode_LED = ON;
				halfHourTicker.detach();
				half_hour_flag=false;
				full_hour_flag=false;
					
			}else if (mode == ADVANCED){ 
				mode = TEST;
				AdvancedMode_LED = OFF;
				TestMode_LED = ON;
			}
		}//user button if	
		switch (mode){
			case TEST:
				TestMode_LED = 1;
				//NormalMode_LED   = 0;
				//AdvancedMode_LED = 0;
				flags_read = event_flags.wait_any(EV_FLAG_READ_SENSORS,0);
			  
				if(flags_read == EV_FLAG_READ_SENSORS){
					event_flags.clear(EV_FLAG_READ_SENSORS);
					//when it's received send event to output thread to print system info:
					//Read mail_data from sensor mailbox (sensors data) and put it in print mailbox
					mail_t *mail_data_sensor = (mail_t *) sensor_data_mail_box.try_get();
					if(mail_data_sensor != NULL){
						set_color_RGB_led(mail_data_sensor->dominant_color);
						put_sensor_data_to_print_mail(mail_data_sensor);
					}
					sensor_data_mail_box.free(mail_data_sensor);
				}
			break;
			case NORMAL:
				flags_read = event_flags.wait_any(EV_FLAG_READ_SENSORS,0);
			  
				if(flags_read == EV_FLAG_READ_SENSORS){
					event_flags.clear(EV_FLAG_READ_SENSORS);
					//when it's received send event to output thread to print system info:
					//Read mail_data from sensor mailbox (sensors data) and put it in print mailbox
					mail_t *mail_data_sensor = (mail_t *) sensor_data_mail_box.try_get();
					if(mail_data_sensor != NULL){
						//Check errors
						checkRange_and_set_RGB_color(mail_data_sensor->temperature,mail_data_sensor->humidity,mail_data_sensor->light,mail_data_sensor->moisture,mail_data_sensor->accel_values,mail_data_sensor->dominant_color);
						//Update log_values with the new recorded values
						updateLog(&log_values,mail_data_sensor->temperature,mail_data_sensor->humidity,mail_data_sensor->light,mail_data_sensor->moisture,mail_data_sensor->dominant_color,mail_data_sensor->accel_values);
						//Receive sensors data from measure_thread and send it to output_thread to print info		
						put_sensor_data_to_print_mail(mail_data_sensor);
					}
					sensor_data_mail_box.free(mail_data_sensor);
				}//flag_read_sensors
				if(half_hour_flag){
					half_hour_flag = false;
					if(full_hour_flag){
						full_hour_flag = false;
						calculate_average_sensors_data(&log_values);
						char dominant_color_system = calculate_dominant_color_from_logs(log_values);
						//send log info to output thread
						put_log_sensor_data_to_print_mail_logs(log_values, dominant_color_system);
						initLog(&log_values);
					}else
						full_hour_flag = true;
					}
			break;
			case ADVANCED:
				
			break;
		}//switch
	}//while
} //main


void half_hour_irq()
{
		half_hour_flag = true;
}
void user_button()
{
		user_button_flag = true;
}

/*
	Normal ranges:
Temperature: 18-28?C
Humidity: 40-60%
Ambient light: 20-100%
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
void checkRange_and_set_RGB_color(float temperature,float humidity,float light_value_f,float moisture_value_f,float accel_values [],char dominantColor){
	if(temperature > TEMPERATURE_MAX || temperature < TEMPERATURE_MIN)
		RGB_LED=0b000;
	else	if(humidity > HUMIDITY_MAX || humidity < HUMIDITY_MIN){
		RGB_LED=0b111;
	}else if(light_value_f > LIGHT_MAX || light_value_f < LIGHT_MIN){
		RGB_LED=0b101;
	}else if(moisture_value_f > MOISTURE_MAX || moisture_value_f < MOISTURE_MIN){
		RGB_LED=0b110;
	}else if(dominantColor != 'G'){
		RGB_LED=0b011;
	}else if( !(accel_values[2] > accel_values[1]) || !(accel_values[2] > accel_values[0])){
		RGB_LED=0b001;
	}else if(dominantColor == 'G'){//If no errors=GREEN, detecting plant
		RGB_LED=0b010;
	}
}

char set_dominant_color(int rgb_readings[4]){
	char dominant_color = 'N';
	if(rgb_readings[1]>rgb_readings[2] && rgb_readings[1]>=rgb_readings[3]){//If max=RED
					//RGB_LED=0b001;
					dominant_color='R'; //red
				}else if(rgb_readings[2]>rgb_readings[1] && rgb_readings[2]>rgb_readings[3]){//If max=Green
					//RGB_LED=0b010;
					dominant_color='G'; //green
				}else if(rgb_readings[3]>rgb_readings[1] && rgb_readings[3]>rgb_readings[2]){//If max=Blue
					//RGB_LED=0b100;
					dominant_color='B';	//blue				
				}else{
					//RGB_LED=0b000;
					dominant_color='N'; //nothing
				}
			return dominant_color;
}

void set_color_RGB_led(char dominant_color){
	if(dominant_color == 'R'){
		RGB_LED=0b001;
	}else if(dominant_color == 'G'){//If max=Green
		RGB_LED=0b010;
	}else if(dominant_color == 'B'){//If max=Blue
		RGB_LED=0b100;	
	}else if(dominant_color == 'N'){ //
		RGB_LED=0b000;
	}
}


/**
* Function put_sensor_data_on_Mailbox
* Description: Reading light sensor data, put in on sensor mailbox 
* and activate event flag EV_FLAG_READ_LIGHT to inform the main thread. 
* Executed each LIGHT_SENSOR_READ_CADENCY (2s) in the eventQueue of measure_thread
*/
void put_sensor_data_on_Mailbox(void)
{
		int rgb_readings[4]; // declare a 4 element array to store RGB sensor readings
		float accel_values [3];
		mail_t *mail_data_sensors = sensor_data_mail_box.try_calloc();
	
		//Temperature and humidity
		tempHumSensor.get_data();
		int32_t temperature = tempHumSensor.get_temperature();//Value is multiplied by 1000
		uint32_t humidity = tempHumSensor.get_humidity();//Value is multiplied by 1000
	  //Temp
		mail_data_sensors->temperature = temperature/1000.0;
	  //Humidity
		mail_data_sensors->humidity = humidity/1000.0;
		//Light sensor
		mail_data_sensors->light = lightSensor.read_u16()*100.0/65536.0;
		//Moisture sensor
		mail_data_sensors->moisture = moistureSensor.read_u16()*100.0/65536.0;
		//RGB sensor
		//RGB green predominant, detecting plant
		rgb_sensor.getAllColors(rgb_readings); // read the sensor to get red, green, and blue color data along with overall brightness
		mail_data_sensors->rgb_readings[0] = rgb_readings[0];		//Clear
		mail_data_sensors->rgb_readings[1] = rgb_readings[1];		//Red
		mail_data_sensors->rgb_readings[2] = rgb_readings[2];		//Green
		mail_data_sensors->rgb_readings[3] = rgb_readings[3];		//Blue
		mail_data_sensors->dominant_color = set_dominant_color(rgb_readings);
		//axis Z plant steady  axis X or axis Y out of range
		//Acceleration sensor
		accel_sensor.getAccAllAxis(accel_values);
		mail_data_sensors->accel_values[0] = accel_values[0]; // Accel x
		mail_data_sensors->accel_values[1] = accel_values[1]; // Accel y
		mail_data_sensors->accel_values[2] = accel_values[2]; // Accel z
		
    sensor_data_mail_box.put(mail_data_sensors);
		event_flags.set(EV_FLAG_READ_SENSORS);		
}



/////////////////////////////////////////////////////////////////////////////////////
/**
* Function put_sensor_data_to_print_mail
* @param mail_t *mail_data_sensor 
* Description:  Get the mail data from sensor mailbox, put in on print mailbox 
* and activate event flag EV_FLAG_PRINT_INFO to print the light value.
*/
void put_sensor_data_to_print_mail(mail_t *mail_data_sens){
	
		mail_t *mail_data_print = print_mail_box.try_calloc();
		//Temp
		mail_data_print->temperature = mail_data_sens->temperature;
		//Humidity
		mail_data_print->humidity = mail_data_sens->humidity;
		//Light
		mail_data_print->light = mail_data_sens->light;
		//Moisture
		mail_data_print->moisture = mail_data_sens->moisture;
		//RGB
		mail_data_print->rgb_readings[0] = mail_data_sens->rgb_readings[0];		//Clear
		mail_data_print->rgb_readings[1] = mail_data_sens->rgb_readings[1];		//Red
		mail_data_print->rgb_readings[2] = mail_data_sens->rgb_readings[2];		//Green
		mail_data_print->rgb_readings[3] = mail_data_sens->rgb_readings[3];		//Blue
		mail_data_print->dominant_color = mail_data_sens->dominant_color;
		//Accelerometer
		mail_data_print->accel_values[0] = mail_data_sens->accel_values[0]; // Accel x
		mail_data_print->accel_values[1] = mail_data_sens->accel_values[1]; // Accel y
		mail_data_print->accel_values[2] = mail_data_sens->accel_values[2]; // Accel z
	
		print_mail_box.put(mail_data_print);
		event_flags.set(EV_FLAG_PRINT_INFO);	
}

/**
* Function put_log_sensor_data_to_print_mail_logs
* @param Log log_values 
* Description:  Get the mail data from sensor mailbox, put in on print mailbox 
* and activate event flag EV_FLAG_PRINT_INFO to print the light value.
*/
void put_log_sensor_data_to_print_mail_logs(Log log_hour_values, char dominant_color){
	
		mail_t_logs *mail_data_print = print_logs_mail_box.try_calloc();
		//Temp
		mail_data_print->log_values.temperature_max = log_hour_values.temperature_max;
		mail_data_print->log_values.temperature_min = log_hour_values.temperature_min;
		mail_data_print->log_values.temperature_avg = log_hour_values.temperature_avg;
		//Humidity
		mail_data_print->log_values.humidity_max = log_hour_values.humidity_max;
		mail_data_print->log_values.humidity_min = log_hour_values.humidity_min;
		mail_data_print->log_values.humidity_avg = log_hour_values.humidity_avg;
		//Light
		mail_data_print->log_values.light_max = log_hour_values.light_max;
		mail_data_print->log_values.light_min = log_hour_values.light_min;
		mail_data_print->log_values.light_avg = log_hour_values.light_avg;
		//Moisture
		mail_data_print->log_values.moisture_max = log_hour_values.moisture_max;
		mail_data_print->log_values.moisture_min = log_hour_values.moisture_min;
		mail_data_print->log_values.moisture_avg = log_hour_values.moisture_avg;
		//Dominant color
		mail_data_print->dominant_color = dominant_color;
		//Accelerometer
		mail_data_print->log_values.accel_x_max = log_hour_values.accel_x_max; // Accel x
		mail_data_print->log_values.accel_x_min = log_hour_values.accel_x_min;
		mail_data_print->log_values.accel_y_max = log_hour_values.accel_y_max; // Accel y
		mail_data_print->log_values.accel_y_min = log_hour_values.accel_y_min;
		mail_data_print->log_values.accel_z_max = log_hour_values.accel_z_max; // Accel z
		mail_data_print->log_values.accel_z_min = log_hour_values.accel_z_min;
																																					
	
		print_logs_mail_box.put(mail_data_print);
		event_flags.set(EV_FLAG_PRINT_INFO_LOGS);	
}

char const* get_str_dominant_color(char dominant_color){
	char const *color_dominant_detected;
	if(dominant_color == 'R'){
		color_dominant_detected = "RED";
				
	}else if (dominant_color == 'G'){
		color_dominant_detected = "GREEN";
				
	}else if (dominant_color == 'B'){
		color_dominant_detected = "BLUE";
				
	}else if (dominant_color == 'N'){
		color_dominant_detected = "NONE";
	}
	return color_dominant_detected;
}
///////////////////////////////////////////////////////////////////////////////////
//MEASURE THREAD 
///////////////////////////////////////////////////////////////////////////////////
void measure_sensors(void){
	
	if(!tempHumSensor.check()){
			serial_mutex.lock();
			printf("Temperature and humidity sensor error");
			serial_mutex.unlock();
		}
	
	//Turn on color sensor
	//Get ENABLE register		
	rgb_sensor.enablePowerAndRGBC();
	rgb_sensor.enableWait();
	rgb_sensor.setWaitTime(1500);
	
	while(true) {
		
		if(mode == TEST || mode == NORMAL){
			put_sensor_data_on_Mailbox();
		}
		if(mode == TEST){
			ThisThread::sleep_for(2s);
		}else if (mode == NORMAL){
			ThisThread::sleep_for(2s);
		} else if (mode == ADVANCED){
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//OUTPUT THREAD 
///////////////////////////////////////////////////////////////////////////////////
/**
* Function print_info_system
* Description: Task executed by the output thread to print system info:
* When it receives the event flag EV_FLAG_PRINT_INFO, retrieves from print mailbox
* the light value, convert it to percentage and print system info.
* "State, season, light_value %, threshold: 41.2%"
*/ 
void GPS_and_print_info_system(void){
	char const *stringsStates[] = {"TEST", "NORMAL", "ADVANCED"};
	uint32_t flags_read_serial_th;
	
	GPS_sensor.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //these commands are defined in MBed_Adafruit_GPS.h; a link is provided there for command creation
  GPS_sensor.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS_sensor.sendCommand(PGCMD_ANTENNA);
  
	while(true){
		//GPS
		char c = GPS_sensor.read();
		//If a NMEA message is received
		if (GPS_sensor.newNMEAreceived()) {
			if (!GPS_sensor.parse(GPS_sensor.lastNMEA()))  // this also sets the newNMEAreceived() flag to false
			// we can fail to parse a sentence in which case we should just wait for another
			//printf("Fail to parse\n");
					;
		}
		flags_read_serial_th = event_flags.wait_any(EV_FLAG_PRINT_INFO | EV_FLAG_PRINT_INFO_LOGS,0);//Wait for flag to send the information
		
		if(flags_read_serial_th == EV_FLAG_PRINT_INFO){
			mail_t *mail_data_info = (mail_t *) print_mail_box.try_get();//Get sensors value
			if (mail_data_info != NULL){
			//Send information
			uint8_t local_time_hour = GPS_sensor.hour + 1; //UTC+1 (Madrid Winter time)
			if(local_time_hour > 23) local_time_hour = 0;
			char const *dominant_color = get_str_dominant_color(mail_data_info->dominant_color);
			serial_mutex.lock();
			printf("GPS: #Sats: %d, Lat(UTC): %f, Long(UTC): %f, Altitude: %.0fm, GPS_time: %d:%d:%d GPS_date: %d/%d/%d\n",GPS_sensor.satellites,GPS_sensor.latitude/100,
				GPS_sensor.longitude/100,GPS_sensor.altitude,local_time_hour,GPS_sensor.minute,GPS_sensor.seconds, GPS_sensor.day, GPS_sensor.month, GPS_sensor.year);
			printf("TEMP/HUM: Temperature: %.1f C, Relative Humidity: %.1f%%\n",mail_data_info->temperature,mail_data_info->humidity);
			printf("ACCELEROMETER: X_axis=%.2fg, Y_axis=%.2fg, Z_axis=%.2fg\n",mail_data_info->accel_values[0],mail_data_info->accel_values[1],mail_data_info->accel_values[2]);
			printf("LIGHT: %.1f%%\n",mail_data_info->light);
			printf("SOIL MOISTURE: %.1f%%\n",mail_data_info->moisture);
			printf("COLOR SENSOR: Clear=%d, Red=%d, Green=%d, Blue=%d   -- Dominant color: %s\n\n",mail_data_info->rgb_readings[0],mail_data_info->rgb_readings[1],mail_data_info->rgb_readings[2],mail_data_info->rgb_readings[3],dominant_color);      
			serial_mutex.unlock();
			print_mail_box.free(mail_data_info);
			event_flags.clear(EV_FLAG_PRINT_INFO);
			}				
		}
		if(flags_read_serial_th == EV_FLAG_PRINT_INFO_LOGS){
			mail_t_logs *mail_data_logs = (mail_t_logs *) print_logs_mail_box.try_get();
			if (mail_data_logs != NULL){
				char const *dominant_color = get_str_dominant_color(mail_data_logs->dominant_color);
				serial_mutex.lock();
				printf("\n\n------------SUMMARY VALUES 1 HOUR---------\n");
				printf("TEMP: Max: %.1f C, Min: %.1f C, Avg %.1f C\n",mail_data_logs->log_values.temperature_max,mail_data_logs->log_values.temperature_min,mail_data_logs->log_values.temperature_avg);
				printf("HUM: Max: %.1f%%, Min: %.1f%%, Avg %.1f%%\n",mail_data_logs->log_values.humidity_max,mail_data_logs->log_values.humidity_min,mail_data_logs->log_values.humidity_avg);
				printf("LIGHT: Max: %.1f%%, Min: %.1f%%, Avg %.1f%%\n",mail_data_logs->log_values.light_max,mail_data_logs->log_values.light_min,mail_data_logs->log_values.light_avg);
				printf("MOISTURE: Max: %.1f%%, Min: %.1f%%, Avg %.1f%%\n",mail_data_logs->log_values.moisture_max,mail_data_logs->log_values.moisture_min,mail_data_logs->log_values.moisture_avg);
				printf("COLOR: Dominant color: %s\n",dominant_color);
				printf("ACCELEROMETER: X: Max: %.1f, Min: %.1f. Y: Max: %.1f, Min: %.1f. Z: Max: %.1f, Min: %.1f.\n",mail_data_logs->log_values.accel_x_max,mail_data_logs->log_values.accel_x_min,mail_data_logs->log_values.accel_y_max,mail_data_logs->log_values.accel_y_min,mail_data_logs->log_values.accel_z_max,mail_data_logs->log_values.accel_z_min);
				printf("----------------------------------------------\n\n");
				serial_mutex.unlock();
				print_logs_mail_box.free(mail_data_logs);
				event_flags.clear(EV_FLAG_PRINT_INFO_LOGS);
			}
		}
	}	
}
