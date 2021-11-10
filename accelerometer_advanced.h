#include "mbed.h"

typedef struct {
		float temperature;
		float humidity;
		float light;
		float moisture;
		float accel_values[3];
	  int rgb_readings[4];
		char dominant_color;
		int count_plant_falls;
} mail_t_advanced;

enum PlantOrientation{UP,DOWN};

typedef struct {
		int count_plant_falls;
		PlantOrientation previousState;
} PlantOrientationLog;


void updatePlantOrientation ( PlantOrientationLog *log, float accel_values[3]);