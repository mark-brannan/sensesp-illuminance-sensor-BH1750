#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <BH1750.h>
#include "my_repeat_sensor.h"



using namespace sensesp;

/*
The ADD pin is used to set sensor I2C address. If it has voltage greater or equal
to 0.7VCC voltage (e.g. you've connected it to VCC) the sensor address will be
0x5C. In other case (if ADD voltage less than 0.7 * VCC) the sensor address
will be 0x23 (by default).
*/
const byte k_BH1750_addr_vcc_low = 0x23;
const byte k_BH1750_addr_vcc_high = 0x5C;
const byte k_BH1750_addr_default = k_BH1750_addr_vcc_low;


class LightSensor : public MyRepeatSensor<float> {
 public:
  LightSensor(
    const char* location = "outside",
    byte addr = k_BH1750_addr_default,
    const char* config_path = "");

  void begin();

  float readLightLevel();

 private:
  BH1750 lightMeter;
  const char* location;
  const byte addr;
};

//static inline const String ConfigSchema(const 
  //return R"###({
//}

//class IllumanceStatusPageItem

#endif // LIGHT_SENSOR_H