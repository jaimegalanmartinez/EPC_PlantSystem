#include "mbed.h"
#include "hardware.h"

#ifndef _ACCELEROMETER_ADVANCED_H_
#define _ACCELEROMETER_ADVANCED_H_

typedef struct {
		float temperature;
		float humidity;
		float light;
		float moisture;
		float accel_values[3];
	  int rgb_readings[4];
		char dominant_color;
		int count_plant_falls;
		uint16_t count_single_taps;
} mail_t_advanced;

enum PlantOrientation{UP,DOWN};

typedef struct {
		int count_plant_falls;
		uint16_t count_single_taps;
		PlantOrientation previousState;
} PlantOrientationLog;


void updatePlantOrientation ( PlantOrientationLog *log, float accel_values[3]);

#endif



