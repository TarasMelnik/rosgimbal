#include "hmc5883l.h"

HMC5883L::HMC5883L(I2C* i2c_drv) {
  i2c = i2c_drv;
}

bool HMC5883L::init() {
  // Wait for the chip to power up
  while (millis() < 500);

  // Detect Magnetometer
  uint8_t byte = 0;
  if(!i2c->read(HMC58X3_ADDR, HMC58X3_ID1, &byte))
  {
    return false;
  }
  else
  {
    // Configure HMC5833L
    i2c->write(HMC58X3_ADDR, HMC58X3_CRA, HMC58X3_CRA_DO_75 | HMC58X3_CRA_NO_AVG | HMC58X3_CRA_MEAS_MODE_NORMAL ); // 75 Hz Measurement, no bias, no averaging
    i2c->write(HMC58X3_ADDR, HMC58X3_CRB, HMC58X3_CRB_GN_390); // 390 LSB/Gauss
    i2c->write(HMC58X3_ADDR, HMC58X3_MODE, HMC58X3_MODE_CONTINUOUS); // Continuous Measurement Mode
    return true;
  }
}

bool HMC5883L::read(float (&mag_data)[3]) {
  uint8_t raw[6] = {0, 0, 0, 0, 0, 0};
  i2c->read(HMC58X3_ADDR, HMC58X3_DATA, 6, raw);
//  i2c->read(HMC58X3_ADDR, HMC58X3_DATA, 6, raw);
  mag_data[0] = (float)((int16_t)((raw[0] << 8) | raw[1]));
  mag_data[1] = (float)((int16_t)((raw[2] << 8) | raw[3]));
  mag_data[2] = (float)((int16_t)((raw[4] << 8) | raw[5]));

  //if the mag's ADC over or underflows, then the data register is given the value of -4096
  //the data register can also be assigned -4096 if there's a math overflow during bias calculation
  return mag_data[0] != -4096 && mag_data[1] != -4096 && mag_data[2] != -4096;
}
