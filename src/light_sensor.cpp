#include "light_sensor.h"

#include <esp_log.h>

#include "sensesp.h"

static constexpr char kTag[] = "LightSensor";

// 1 sec seems to work fine, but the interval is also web-configurable.
const unsigned int kDefaultReadIntervalMs = 1000;

LightSensor::LightSensor(const char* location, byte addr,
                         const String& config_path)
    : FloatSensor(config_path),
      light_meter_(addr),
      location_(location),
      addr_(addr),
      repeat_interval_ms_(kDefaultReadIntervalMs) {
  this->load();
  reschedule();
}

void LightSensor::set_repeat_interval_ms(unsigned int repeat_interval_ms) {
  repeat_interval_ms_ = repeat_interval_ms;
  reschedule();
}

void LightSensor::reschedule() {
  if (repeat_event_ != nullptr) {
    repeat_event_->remove(event_loop());
  }
  repeat_event_ = event_loop()->onRepeat(
      repeat_interval_ms_, [this]() { this->emit(this->read_light_level()); });
}

void LightSensor::begin() {
  // 'one time' mode is preferrable because it can sleep between reads,
  // as opposed to the continuous mode, which reads every 120ms.
  if (light_meter_.begin(BH1750::ONE_TIME_HIGH_RES_MODE, addr_)) {
    ESP_LOGD(kTag, "BH1750 for '%s' initialized with address %x",
             location_.c_str(), addr_);
  } else {
    ESP_LOGE(kTag, "BH1750 for '%s' initialization failed", location_.c_str());
  }

  this->attach([this]() {
    ESP_LOGD(kTag, "%s light sensor value: %f lx", location_.c_str(),
             this->get());
  });
}

float LightSensor::read_light_level() {
  while (!light_meter_.measurementReady(true)) {
    yield();
  }
  float lux = light_meter_.readLightLevel();
  adjust_MTreg(lux);
  return lux;
}

// See the datasheet as well as the claws/BH1750 examples
// https://www.elechouse.com/elechouse/images/product/Digital%20light%20Sensor/bh1750fvi-e.pdf
// https://github.com/claws/BH1750/blob/master/examples/BH1750autoadjust/BH1750autoadjust.ino
// https://thecavepearlproject.org/2024/08/10/using-a-bh1750-lux-sensor-to-measure-par/

const byte k_min_MTreg = 0x1F;      // 31
const byte k_default_MTreg = 0x45;  // 69
const byte k_high_MTreg = 0x8A;     // 138
const byte k_max_MTreg = 0xFE;      // 254

enum LightLevel {
  ERROR = -1,
  VERY_LOW_LIGHT = 0,
  LOW_LIGHT = 1,
  NORMAL_LIGHT = 10,
  DIRECT_SUN = 40000
};

static LightLevel to_light_level(float lux) {
  if (lux > 40000.0) {
    return DIRECT_SUN;
  } else if (lux > 10.0) {
    return NORMAL_LIGHT;
  } else if (lux > 1.0) {
    return LOW_LIGHT;
  } else if (lux > 0.0) {
    return VERY_LOW_LIGHT;
  } else {
    return ERROR;
  }
}

void LightSensor::adjust_MTreg(float lux) {
  ESP_LOGD(kTag,
           "Adjusting MTreg for %s sensor; current lux=%f, prior=%f, MTreg=%u",
           location_.c_str(), lux, prior_lux_, prior_MTreg_);

  prior_lux_ = lux;
  byte new_MTreg = k_default_MTreg;

  switch (to_light_level(lux)) {
    case DIRECT_SUN:
      new_MTreg = k_min_MTreg;
      ESP_LOGD(kTag,
               "Setting low MTreg (%X/%u) for high light environment (%f lx)",
               new_MTreg, new_MTreg, lux);
      light_meter_.setMTreg(new_MTreg);
      light_meter_.configure(BH1750::ONE_TIME_LOW_RES_MODE);
      break;

    case NORMAL_LIGHT:
      new_MTreg = k_default_MTreg;
      ESP_LOGD(kTag,
               "Setting default MTreg (%X/%u) for normal light environment "
               "(%f lx)",
               new_MTreg, new_MTreg, lux);
      light_meter_.setMTreg(new_MTreg);
      light_meter_.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
      break;

    case LOW_LIGHT:
      new_MTreg = k_high_MTreg;
      ESP_LOGD(kTag,
               "Setting high MTreg (%X/%u) for low light environment (%f lx)",
               new_MTreg, new_MTreg, lux);
      light_meter_.setMTreg(new_MTreg);
      light_meter_.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
      break;

    case VERY_LOW_LIGHT:
      new_MTreg = k_max_MTreg;
      ESP_LOGD(kTag,
               "Setting max MTreg (%X/%u) for very low light environment "
               "(%f lx)",
               new_MTreg, new_MTreg, lux);
      light_meter_.setMTreg(new_MTreg);
      light_meter_.configure(BH1750::ONE_TIME_HIGH_RES_MODE_2);
      break;

    case ERROR:
      ESP_LOGE(kTag, "Error condition for sensor; lux is negative: %f", lux);
      light_meter_.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
      break;

    default:
      ESP_LOGE(kTag, "Unknown light level: %f", lux);
      light_meter_.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
      break;
  }
  prior_MTreg_ = new_MTreg;
}
