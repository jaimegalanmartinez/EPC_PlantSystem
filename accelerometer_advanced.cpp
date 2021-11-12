#include "mbed.h"
#include "hardware.h"
#include "accelerometer_advanced.h"




/*When configured for a single tap event, an interrupt is generated when the input acceleration on the selected axis exceeds
the programmed threshold, and returns below it within a time window defined by PULSE_TMLT. If the ELE bit (bit 6) of the
PULSE_CFG (Reg 0x21) register is not set, the interrupt is kept high for the duration of the Latency window PULSE_LTCY (Reg
0x27). The latency window is a user-definable period of delay after each pulse. This latency window applies either for single pulse
or double pulse
 If the ELE bit is set, the source register values will remain static until the PULSE_SRC (Reg 0x22) register is read
*/
//readRegs(int addr, uint8_t * data, int len)
void config_single_tap_event(){
	uint8_t threshold_value = 
	//Thresholds can be changed in either standby or active mode
	//Set tap detection threshold for 2g on Z
	//2g/0.063g/count = 32 counts 0x20
	//3g/0.063g/count = 48 counts 0x30
	accel_sensor.setThreshold_z_tap();
}
void detect_tap_event(float accel_value_x, float accel_value_y, float accel_value_z){
	
	if(accel_value_x > accel_sensor.getThreshold_x_tap()){
		//tap detected in axis x
	}
	if(accel_value_y > accel_sensor.getThreshold_y_tap()){
		//tap detected in axis x
	}
	if(accel_value_z > accel_sensor.getThreshold_z_tap()){
		//tap detected in axis x
	}
}
