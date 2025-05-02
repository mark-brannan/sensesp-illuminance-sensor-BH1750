#include "light_sensor.h"
#include "sensesp.h"
#include "sensesp/ui/status_page_item.h"


// 1 sec seems to work fine, but the repeat sensor class is also configurable.
const unsigned int default_read_interval = 1000;

LightSensor::LightSensor(const char* location, byte addr, const char* config_path)
  : lightMeter(addr),
    location(location),
    addr(addr),
    MyRepeatSensor<float>(default_read_interval, [this]() { return readLightLevel(); }, config_path)
{
  ConfigItem(this)->set_title(String(location) + " light sensor");
};

void LightSensor::begin() {
  // 'one time' mode is preferrable because it can sleep between reads,
  // as opposed to the continuous mode, which reads every 120ms.
  if (lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE, addr)) {
    debugD("BH1750 for '%s' initialized with address %x", location, addr);
  } else {
    debugE("BH1750 for '%s' initialization failed", location);
  }

  sensesp::Observable::attach([this]() {
    debugD("%s light sensor value: %f lx", location, this->get());
  });
};

float LightSensor::readLightLevel() {
    while (!lightMeter.measurementReady(true)) {
      debugD("Waiting for %s light measurement to be ready", location);
      yield();
    }
    float lux = lightMeter.readLightLevel();
    lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
    return lux;
  };
