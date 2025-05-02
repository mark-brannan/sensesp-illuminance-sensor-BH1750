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

#include "sensesp.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/ui/status_page_item.h"
#include "sensesp/sensors/constant_sensor.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/transforms/frequency.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp/transforms/time_counter.h"
#include "sensesp_app_builder.h"
#include "light_sensor.h"
#include "illuminance.h"

using namespace sensesp;


auto retained = std::vector<std::shared_ptr<void>>();


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

  auto outside_sensor = std::make_shared<LightSensor>(
    "outside",
    k_BH1750_addr_vcc_low,
    "/Sensors/Outside Light Sensor");

  auto inside_sensor = std::make_shared<LightSensor>(
    "inside",
    k_BH1750_addr_vcc_high,
    "/Sensors/Inside Light Sensor");

  outside_sensor->begin();
  inside_sensor->begin();

  auto sk_output_outside = std::make_shared<SKOutputIlluminance>("outside");
  auto sk_output_inside = std::make_shared<SKOutputIlluminance>("inside");

  outside_sensor->connect_to(sk_output_outside);
  inside_sensor->connect_to(sk_output_inside);

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
