/* Copyright (c) 2010-2011 mbed.org, MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*	https://os.mbed.com/users/emilmont/code/MMA8451Q/
*/
 
#ifndef MMA8451Q_H
#define MMA8451Q_H
 
#include "mbed.h"
 
/**
* MMA8451Q accelerometer example
* #include "mbed.h"
* #include "MMA8451Q.h"
*
* #define MMA8451_I2C_ADDRESS (0x1d<<1)
*
* int main(void) {
*    DigitalOut led(LED_GREEN);
*    MMA8451Q acc(P_E25, P_E24, MMA8451_I2C_ADDRESS);
*    printf("WHO AM I: 0x%2X\r\n", acc.getWhoAmI());
*    
*    while (true) {
*        printf("-----------\r\n");
*        printf("acc_x: %d\r\n", acc.getAccX());
*        printf("acc_y: %d\r\n", acc.getAccY());
*        printf("acc_z: %d\r\n", acc.getAccZ());
*        
*        wait(1);
*        led = !led;
*    }
* }
*/
class MMA8451Q
{
public:
  /**
  * MMA8451Q constructor
  *
  * @param sda SDA pin
  * @param sdl SCL pin
  * @param addr addr of the I2C peripheral
  */
  MMA8451Q(PinName sda, PinName scl, int addr);
 
  /**
  * MMA8451Q destructor
  */
  ~MMA8451Q();
 
  /**
   * Get the value of the WHO_AM_I register
   *
   * @returns WHO_AM_I value
   */
  uint8_t getWhoAmI();
 
  /**
   * Get X axis acceleration
   *
   * @returns X axis acceleration
   */
  float getAccX();
 
  /**
   * Get Y axis acceleration
   *
   * @returns Y axis acceleration
   */
  float getAccY();
	
	/**
   * Get XYZ axis acceleration
   *
   * @param res array where acceleration data will be stored
   */
  void getAccAllAxis(float * res);
 
  /**
   * Get Z axis acceleration
   *
   * @returns Z axis acceleration
   */
  float getAccZ();
 
  /**
   * Get the value of the PULSE_THSX register
   *
   * @returns PULSE_THSX value
   */
 uint8_t getThreshold_x_tap();
 
 /**
   * Get the value of the PULSE_THSY register
   *
   * @returns PULSE_THSY value
   */
 uint8_t getThreshold_y_tap();
 
 /**
   * Get the value of the PULSE_THSZ register
   *
   * @returns PULSE_THSZ value
   */
 uint8_t getThreshold_z_tap();
 
 /**
   * Set the value of the PULSE_THSX register
	 *	2g/0.063g/count = 32 counts 0x20
	 *	3g/0.063g/count = 48 counts 0x30
	 *  BIT 7 is always 0
   */
 void setThreshold_x_tap(uint8_t threshold_value);
  /**
   * Set the value of the PULSE_THSY register
	 *	2g/0.063g/count = 32 counts
	 *	3g/0.063g/count = 48 counts
	 *  BIT 7 is always 0
   */
 void setThreshold_y_tap(uint8_t threshold_value);
  /**
   * Set the value of the PULSE_THSZ register
	 *	2g/0.063g/count = 32 counts
	 *	3g/0.063g/count = 48 counts
	 *  BIT 7 is always 0
   */
 void setThreshold_z_tap(uint8_t threshold_value);
  /*
	 * Configures the sensor to detect freefalls
	 */
	void initFreeFall();
	/**
	 * Configures the sensor to not detect freefalls
	 */
	void uninitFreeFall();
	/**
	 * Reads if there was a freefall
	 * Return: true if freefall happened
	 */
	bool getFF();
 
private:
  I2C m_i2c;
  int m_addr;
  void readRegs(int addr, uint8_t * data, int len);
  void writeRegs(uint8_t * data, int len);
  int16_t getAccAxis(uint8_t addr);
 
};
 
#endif