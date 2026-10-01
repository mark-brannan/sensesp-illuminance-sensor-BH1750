#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>
#include <BH1750.h>

#include "ReactESP.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/ui/config_item.h"

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

/**
 * @brief BH1750 light sensor read at a (web-configurable) repeat interval.
 *
 * Upstream RepeatSensor has no config path and no ConfigSchema, so the repeat
 * scheduling is implemented here directly (the same pattern upstream sensors
 * such as AnalogInput use) instead of subclassing RepeatSensor.
 *
 * Register with ConfigItem(sensor) after construction to expose the interval
 * in the web UI. The saved config key is "repeat_interval_ms".
 */
class LightSensor : public FloatSensor {
 public:
  LightSensor(const char* location = "outside",
              byte addr = k_BH1750_addr_default,
              const String& config_path = "");

  virtual ~LightSensor() {
    if (repeat_event_ != nullptr) {
      repeat_event_->remove(event_loop());
    }
  }

  void begin();

  float read_light_level();

  const String& get_location() const { return location_; }

  void set_repeat_interval_ms(unsigned int repeat_interval_ms);

  virtual bool to_json(JsonObject& doc) override {
    doc["repeat_interval_ms"] = repeat_interval_ms_;
    return true;
  }

  virtual bool from_json(const JsonObject& config) override {
    if (!config["repeat_interval_ms"].is<unsigned int>()) {
      return false;
    }
    set_repeat_interval_ms(config["repeat_interval_ms"].as<unsigned int>());
    return true;
  }

 private:
  BH1750 light_meter_;
  String location_;
  const byte addr_;
  unsigned int repeat_interval_ms_;
  reactesp::RepeatEvent* repeat_event_ = nullptr;
  float prior_lux_ = -2.;

  // MTreg is "Measurement Time Register" and is used to set the sensitivity
  // of the sensor.
  byte prior_MTreg_ = 0x45;  // 69, default MTreg value

  void reschedule();
  void adjust_MTreg(float lux);
};

// TODO: should any calibration values be configurable?
inline const String ConfigSchema(const LightSensor& obj) {
  return R"###({"type":"object","properties":{"repeat_interval_ms":{"title":"Repeat interval (ms)","type":"integer"}}})###";
}

#endif  // LIGHT_SENSOR_H
