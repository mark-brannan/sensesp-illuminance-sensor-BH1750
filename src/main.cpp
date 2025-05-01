// Signal K application template file.
//
// This application demonstrates core SensESP concepts in a very
// concise manner. You can build and upload the application as is
// and observe the value changes on the serial port monitor.
//
// You can use this source file as a basis for your own projects.
// Remove the parts that are not relevant to you, and add your own code
// for external hardware libraries.

#include <memory>
#include <Wire.h>
#include <BH1750.h>

#include "sensesp.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/ui/status_page_item.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

/*
The ADD pin is used to set sensor I2C address. If it has voltage greater or equal
to 0.7VCC voltage (e.g. you've connected it to VCC) the sensor address will be
0x5C. In other case (if ADD voltage less than 0.7 * VCC) the sensor address
will be 0x23 (by default).
*/
const byte defaultAddr = 0x23;
const byte vccHighAddr = 0x5C;

BH1750 lightMeterOutside(defaultAddr);
BH1750 lightMeterInside(vccHighAddr);

auto retained = std::vector<std::shared_ptr<void>>();

// These are well-defined paths in the Signal K specification, but a different path could be used
// if you want to do some transformation or remapping in signalk plugin.
// https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpenvironmentinsideilluminance
// https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpenvironmentoutsideilluminance
const char* sk_path_inside_illuminance = "environment.inside.illuminance";
const char* sk_path_outside_illuminance = "environment.outside.illuminance";
const char* config_path_inside_illuminance = "/config/output/inside/illuminance";
const char* config_path_outside_illuminance = "/config/output/outside/illuminance";

unsigned int read_interval = 2000;

void setup() {
  SetupLogging(ESP_LOG_DEBUG);

  SensESPAppBuilder builder;
  sensesp_app = (&builder)
    ->set_hostname("sensesp-illuminance-sensor")
    ->set_wifi_access_point("sensesp-illuminance-sensor", "thisisfine")
    ->get_app();

  // Initialize the I2C bus (BH1750 library doesn't do this automatically).
  // The default pins for I2C are SDA=GPIO21 and SCL=GPIO22.
  if (Wire.begin(SDA, SCL)) {
    debugD("I2C bus initialized with default pins; SDA: %d, SCL: %d", SDA, SCL);
  } else {
    debugE("I2C bus initialization failed");
  }

  // 'one time' mode is preferrable because it can sleep between reads,
  // as opposed to the continuous mode, which reads every 120ms.
  if (lightMeterOutside.begin(BH1750::ONE_TIME_HIGH_RES_MODE, defaultAddr)) {
    debugD("Outside BH1750 initialized with address %x", defaultAddr);
  } else {
    debugE("Outside BH1750 initialization failed");
  }

  if (lightMeterInside.begin(BH1750::ONE_TIME_HIGH_RES_MODE, vccHighAddr)) {
    debugD("Inside BH1750 initialized with address %x", vccHighAddr);
  } else {
    debugE("Inside BH1750 initialization failed");
  }
  auto read_lux_outside = []() {
    while (!lightMeterOutside.measurementReady(true)) {
      debugD("Waiting for measurement to be ready");
      yield();
    }
    float lux = lightMeterOutside.readLightLevel();
    lightMeterOutside.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
    return lux;
  };

  auto read_lux_inside = []() {
    while (!lightMeterInside.measurementReady(true)) {
      debugD("Waiting for measurement to be ready");
      yield();
    }
    float lux = lightMeterInside.readLightLevel();
    lightMeterInside.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
    return lux;
  };

  auto outside_sensor = std::make_shared<RepeatSensor<float>>(read_interval, read_lux_outside);
  auto inside_sensor = std::make_shared<RepeatSensor<float>>(read_interval, read_lux_inside);

  outside_sensor->attach([outside_sensor]() {
    debugD("Outside sensor value: %f lx", outside_sensor->get());
  });

  inside_sensor->attach([inside_sensor]() {
    debugD("Inside sensor value: %f lx", inside_sensor->get());
  });

  auto sk_output_outside = std::make_shared<SKOutputFloat>(
    sk_path_outside_illuminance,
    config_path_outside_illuminance,
    "lux"
  );

  auto sk_output_inside = std::make_shared<SKOutputFloat>(
    sk_path_inside_illuminance,
    config_path_inside_illuminance,
    "lux"
  );

  outside_sensor->connect_to(sk_output_outside);
  inside_sensor->connect_to(sk_output_inside);

  auto makeStatusPageItemFor = [](const char* name, int order) {
    auto status_page_item = std::make_shared<StatusPageItem<float>>(name, -1., "Illuminance Sensors", order);
    retained.push_back(status_page_item);
    return status_page_item;
  };

  sk_output_outside->connect_to(makeStatusPageItemFor("Value sent to SK for 'environment.outside.illuminance'", 1));
  sk_output_inside->connect_to(makeStatusPageItemFor("Value sent to SK for 'environment.inside.illuminance'", 2));

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }