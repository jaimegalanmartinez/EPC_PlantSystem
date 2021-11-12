#include "MMA8451Q.h"
 
#define REG_WHO_AM_I      0x0D
#define REG_CTRL_REG_1    0x2A
#define REG_OUT_X_MSB     0x01
#define REG_OUT_Y_MSB     0x03
#define REG_OUT_Z_MSB     0x05
 
#define UINT14_MAX        16383

/*Configures the event flag for the tap detection interrupt function for enabling/disabling single and double pulse on
 *each of the axes
					Tap Enable
					Bit 7 Bit 6 Bit 5 Bit 4 Bit 3 Bit 2 Bit 1 Bit 0
					DPA ELE ZDPEFE ZSPEFE YDPEFE YSPEFE XDPEFE XSPEFE
Single Tap 0 1 0 1 0 1 0 1 -> 0x55 ALL AXES
Double Tap 0 1 1 0 1 0 1 0 -> 0x6A
Both S & D 0 1 1 1 1 1 1 1 ->
*/
#define REG_PULSE_CFG	0x21 //ELE BIT 6

//With bit ELE activated
#define SINGLE_TAP 0x55 //in all axes 
#define SINGLE_TAP_X 0x41
#define SINGLE_TAP_Y 0x44
#define SINGLE_TAP_Z 0x50
#define DOUBLE_TAP 0x6A // in all axes
#define BOTH_TAPS  0x7F
//Without bit ELE
#define SINGLE_TAP_NO_ELE 0x15 //in all axes
/*Threshold values to trigger tap event
 *The threshold values range from 0 to 127 (7 bits expressed as an absolute value) with
 *steps of 0.063g/LSB at a fixed 8g acceleration range
*/
#define REG_PULSE_THSX 0x23 //threshold_value_x
#define REG_PULSE_THSY 0x24 //threshold_value_y
#define REG_PULSE_THSZ 0x25 //threshold_value_z
#define REG_PULSE_LTCY 0x27//Latency time to hold the event conditions

#define REG_PULSE_TMLT 0x26 //time window, time limit register
/*When the low pass filter is enabled the time step doubles.
The filter should help eliminate additional ringing after the tap signature is detected.
*/
#define REG_LOW_PASS_FILTER 0x0F

MMA8451Q::MMA8451Q(PinName sda, PinName scl, int addr) : m_i2c(sda, scl), m_addr(addr) {
    // activate the peripheral
    uint8_t data[2] = {REG_CTRL_REG_1, 0x01};
    writeRegs(data, 2);
}
 
MMA8451Q::~MMA8451Q() { }
 
uint8_t MMA8451Q::getWhoAmI() {
    uint8_t who_am_i = 0;
    readRegs(REG_WHO_AM_I, &who_am_i, 1);
    return who_am_i;
}
 
float MMA8451Q::getAccX() {
    return (float(getAccAxis(REG_OUT_X_MSB))/4096.0);
}
 
float MMA8451Q::getAccY() {
    return (float(getAccAxis(REG_OUT_Y_MSB))/4096.0);
}
 
float MMA8451Q::getAccZ() {
    return (float(getAccAxis(REG_OUT_Z_MSB))/4096.0);
}
 
void MMA8451Q::getAccAllAxis(float * res) {
    res[0] = getAccX();
    res[1] = getAccY();
    res[2] = getAccZ();
}
 
int16_t MMA8451Q::getAccAxis(uint8_t addr) {
    int16_t acc;
    uint8_t res[2];
    readRegs(addr, res, 2);
 
    acc = (res[0] << 6) | (res[1] >> 2);
    if (acc > UINT14_MAX/2)
        acc -= UINT14_MAX;
 
    return acc;
}
 
void MMA8451Q::readRegs(int addr, uint8_t * data, int len) {
    char t[1] = {(char)addr};
    m_i2c.write(m_addr, t, 1, true);
    m_i2c.read(m_addr, (char *)data, len);
}
 
void MMA8451Q::writeRegs(uint8_t * data, int len) {
    m_i2c.write(m_addr, (char *)data, len);
}

uint8_t MMA8451Q::getThreshold_x_tap() {
	uint8_t threshold_x_read = 0;
	readRegs(REG_PULSE_THSX,&threshold_x_read, 1);
  return threshold_x_read;
}

uint8_t MMA8451Q::getThreshold_y_tap() {
  uint8_t threshold_y_read = 0;
	readRegs(REG_PULSE_THSY,&threshold_y_read, 1);
  return threshold_y_read;
}

uint8_t MMA8451Q::getThreshold_z_tap() {
   uint8_t threshold_z_read = 0;
	readRegs(REG_PULSE_THSZ,&threshold_z_read, 1);
  return threshold_z_read;
}

void MMA8451Q::setThreshold_x_tap(uint8_t threshold_value) {
	uint8_t data[2] = {REG_PULSE_THSX, threshold_value};
  writeRegs(data, 2);
}

void MMA8451Q::setThreshold_y_tap(uint8_t threshold_value) {
  uint8_t data[2] = {REG_PULSE_THSY, threshold_value};
  writeRegs(data, 2);
}

void MMA8451Q::setThreshold_z_tap(uint8_t threshold_value) {
  uint8_t data[2] = {REG_PULSE_THSZ, threshold_value};
  writeRegs(data, 2);
}

