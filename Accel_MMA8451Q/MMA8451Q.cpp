#include "MMA8451Q.h"
 
#define REG_WHO_AM_I      0x0D
#define REG_CTRL_REG_1    0x2A
#define REG_OUT_X_MSB     0x01
#define REG_OUT_Y_MSB     0x03
#define REG_OUT_Z_MSB     0x05
 
#define FF_MT_CFG      0x15
#define FF_MT_SRC       0x16
#define FF_MT_THS        0x17
#define FF_MT_COUNT         0x18
#define FF_MT_CFG      0x15
#define CTRL_REG4      0x2D
#define CTRL_REG5      0x2E
#define UINT14_MAX        16383
 
MMA8451Q::MMA8451Q(PinName sda, PinName scl, int addr) : m_i2c(sda, scl), m_addr(addr) {
    // activate the peripheral
    uint8_t data[2] = {REG_CTRL_REG_1, 0x01};//0x01 but 0x19
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
/*copy
// activate free fall
	uint8_t data0[2] = {REG_CTRL_REG_1, 0x20};//0x19
  writeRegs(data0, 2);
  uint8_t data[2] = {FF_MT_CFG, 0b10111000};
	writeRegs(data,2);
	uint8_t data2[2] = {FF_MT_THS, 0b00000011};
	writeRegs(data2,2);
	uint8_t data5[2] = {FF_MT_COUNT, 0x06};
	writeRegs(data5,2);
	uint8_t data4[2] = {CTRL_REG4, 0x04};
	writeRegs(data4,2);
	uint8_t data44[2] = {CTRL_REG5, 0x00};
	writeRegs(data44,2);
  uint8_t data6[2] = {REG_CTRL_REG_1, 0x01};//0x19
  writeRegs(data6, 2);
	*/
void MMA8451Q::initFreeFall(){
	// activate free fall
	/*uint8_t data0[2] = {REG_CTRL_REG_1, 0x01};//0x19
  writeRegs(data0, 2);*/
  uint8_t data[2] = {FF_MT_CFG, 0b10111000};
	writeRegs(data,2);
	uint8_t data2[2] = {FF_MT_THS, 0b00000011};
	writeRegs(data2,2);
	/*uint8_t data3[2] = {FF_MT_COUNT, 0x0A};
	writeRegs(data3,2);*/
	uint8_t data5[2] = {FF_MT_COUNT, 0x06};
	writeRegs(data5,2);
	/*uint8_t data4[2] = {CTRL_REG4, 0x04};
	writeRegs(data4,2);
	uint8_t data44[2] = {CTRL_REG5, 0x00};
	writeRegs(data44,2);*/
  /*uint8_t data6[2] = {REG_CTRL_REG_1, 0x01};//0x19
  writeRegs(data6, 2);*/
}
void MMA8451Q::uninitFreeFall(){
	// activate free fall
  uint8_t data[2] = {FF_MT_CFG, 0b00000000};
	writeRegs(data,2);
	uint8_t data2[2] = {FF_MT_THS, 0b00000000};
	writeRegs(data2,2);
}
bool MMA8451Q::getFF(){
	// activate free fall
  uint8_t data[1] = {};
	readRegs(FF_MT_SRC,data,1);
	return (data[0] & 0b10000000);
	/*uint8_t data2[2] = {FF_MT_THS, 0b00000110};
	writeRegs(data2,2);
	uint8_t data3[2] = {FF_MT_COUNT, 0x06};
	writeRegs(data3,2);*/
}