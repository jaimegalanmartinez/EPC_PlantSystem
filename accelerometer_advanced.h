#include "mbed.h"
#include "hardware.h"

#ifndef _ACCELEROMETER_ADVANCED_H_
#define _ACCELEROMETER_ADVANCED_H_

typedef struct {
	uint8_t count_single_taps;
	uint8_t x_single_taps;
	uint8_t y_single_taps;
	uint8_t z_single_taps;
} PlantTaps;

typedef struct {
		float temperature;
		float humidity;
		float light;
		float moisture;
		float accel_values[3];
	  int rgb_readings[4];
		char dominant_color;
		uint8_t count_plant_falls;
		PlantTaps plantTaps;
} mail_t_advanced;

enum PlantOrientation{UP,DOWN};

typedef struct {
		uint8_t count_plant_falls;
		PlantOrientation previousState;
} PlantOrientationLog;


void updatePlantOrientation ( PlantOrientationLog *log, float accel_values[3]);

#endif



