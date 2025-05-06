#include "sensesp.h"
#include "light_sensor.h"


// 1 sec seems to work fine, but the repeat sensor class is also configurable.
const unsigned int default_read_interval = 1000;

LightSensor::LightSensor(const char* location, byte addr, const char* config_path)
  : lightMeter(addr),
    location(location),
    addr(addr),
    MyRepeatSensor<float>(default_read_interval, [this]() { return readLightLevel(); }, config_path),
    config_title(this->location + " light sensor") {
  ConfigItem(this)->set_title(config_title);
  this->load();
};

void LightSensor::begin() {
  // 'one time' mode is preferrable because it can sleep between reads,
  // as opposed to the continuous mode, which reads every 120ms.
  if (lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE, addr)) {
    debugD("BH1750 for '%s' initialized with address %x", location.c_str(), addr);
  } else {
    debugE("BH1750 for '%s' initialization failed", location.c_str());
  }

  sensesp::Observable::attach([this]() {
    debugD("%s light sensor value: %f lx", location.c_str(), this->get());
  });
};

float LightSensor::readLightLevel() {
    while (!lightMeter.measurementReady(true)) {
      yield();
    }
    float lux = lightMeter.readLightLevel();
    adjust_MTreg(lux);
    return lux;
  };


// See the datasheet as well as the claws/BH1750 examples
// https://www.elechouse.com/elechouse/images/product/Digital%20light%20Sensor/bh1750fvi-e.pdf
// https://github.com/claws/BH1750/blob/master/examples/BH1750autoadjust/BH1750autoadjust.ino
// https://thecavepearlproject.org/2024/08/10/using-a-bh1750-lux-sensor-to-measure-par/

const byte k_min_MTreg     = 0x1F; // 31
const byte k_default_MTreg = 0x45; // 69
const byte k_high_MTreg    = 0x8A; // 138
const byte k_max_MTreg     = 0xFE; // 254

enum LightLevel {
  ERROR           = -1,
  VERY_LOW_LIGHT  = 0,
  LOW_LIGHT       = 1,
  NORMAL_LIGHT    = 10,
  DIRECT_SUN      = 40000
};

LightLevel toLightLevel(float lux) {
  if (lux > 40000.0) {
    return DIRECT_SUN;
  } else if (lux > 10.0) {
    return NORMAL_LIGHT;
  } else if (lux > 1.0) {
    return LOW_LIGHT;
  } else if (lux > 0.00000) {
    return VERY_LOW_LIGHT;
  } else { return ERROR; }
}

void LightSensor::adjust_MTreg(float lux) {
  debugD("Adjusting MTreg for %s sensor; current lux=%f, prior=%f, MTreg=%u",
      location.c_str(), lux, prior_lux, prior_MTreg);

  prior_lux = lux;
  byte new_MTreg = k_default_MTreg;

  LightLevel level = toLightLevel(lux);
  switch (level) {
    case DIRECT_SUN:
      new_MTreg = k_min_MTreg;
      debugD("Setting low MTreg (%X/%u) for high light environment (%f lx)",
          new_MTreg, new_MTreg, lux);

      lightMeter.setMTreg(new_MTreg);
      lightMeter.configure(BH1750::ONE_TIME_LOW_RES_MODE);
      break;

    case NORMAL_LIGHT:
      new_MTreg = k_default_MTreg;
      debugD("Setting default MTreg (%X/%u) for normal light environment (%f lx)",
        new_MTreg, new_MTreg, lux);

      lightMeter.setMTreg(new_MTreg);
      lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
      break;

    case LOW_LIGHT:
      new_MTreg = k_high_MTreg;
      debugD("Setting high MTreg (%X/%u) for low light environment (%f lx)",
        new_MTreg, new_MTreg, lux);

      lightMeter.setMTreg(k_max_MTreg);
      lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
      break;

    case VERY_LOW_LIGHT:
      new_MTreg = k_max_MTreg;
      debugD("Setting max MTreg (%X/%u) for very low light environment (%f lx)",
        new_MTreg, new_MTreg, lux);

      lightMeter.setMTreg(new_MTreg);
      lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE_2);
      break;

    case ERROR:
      debugE("Error condition for sensor; lux is negative: %f", lux);
      lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
      break;

    default:
      debugE("Unknown light level: %f", lux);
      lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
      break;
  }
  prior_MTreg = new_MTreg;
};