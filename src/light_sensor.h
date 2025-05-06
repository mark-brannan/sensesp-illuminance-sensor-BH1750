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

  virtual bool to_json(JsonObject& doc) override {
    doc["repeat_interval_ms"] = repeat_interval_ms_;
    return true;
  }

  virtual bool from_json(const JsonObject& config) override {
    // configuration parameters non-mandatory
    if (!config["repeat_interval_ms"].is<unsigned int>()) {
      return false;
    }
    set_repeat_interval_ms(config["repeat_interval_ms"].as<unsigned int>());
    return true;
  }

 private:
  BH1750 lightMeter;
  String location;
  const byte addr;
  float prior_lux = -2.;
  String config_title;

  void adjust_MTreg(float lux);

  // MTreg is "Measurement Time Register" and is used to set the sensitivity of the sensor
  byte prior_MTreg = 0x45; // 69, default MTreg value
};

// TODO: should any calibration values be configurable?
inline const String ConfigSchema(const LightSensor& obj) {
  return R"###({"type":"object","properties":{"repeat_interval_ms":{"title":"Repeat interval (ms)","type":"integer"}}})###";
}

#endif // LIGHT_SENSOR_H