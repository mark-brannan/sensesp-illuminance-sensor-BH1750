// SensESP application reading two BH1750 illuminance sensors ("inside" and
// "outside") over I2C and publishing them to Signal K.

#include <Wire.h>
#include <esp_log.h>

#include <memory>

#include "illuminance.h"
#include "light_sensor.h"
#include "sensesp.h"
#include "sensesp/ui/config_item.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

static constexpr char kTag[] = "main";

void setup() {
  SetupLogging(ESP_LOG_DEBUG);

  SensESPAppBuilder builder;
  sensesp_app = (&builder)
                    ->set_hostname("sensesp-illuminance-sensor")
                    ->set_wifi_access_point("sensesp-illuminance-sensor",
                                            "thisisfine")
                    ->get_app();

  // Initialize the I2C bus (BH1750 library doesn't do this automatically).
  // The default pins for I2C are SDA=GPIO21 and SCL=GPIO22.
  if (Wire.begin(SDA, SCL)) {
    ESP_LOGD(kTag, "I2C bus initialized with default pins; SDA: %d, SCL: %d",
             SDA, SCL);
  } else {
    ESP_LOGE(kTag, "I2C bus initialization failed");
  }

  auto outside_sensor = std::make_shared<LightSensor>(
      "outside", k_BH1750_addr_vcc_low, "/Sensors/Outside Light Sensor");
  ConfigItem(outside_sensor)
      ->set_title(outside_sensor->get_location() + " light sensor")
      ->set_sort_order(100);

  auto inside_sensor = std::make_shared<LightSensor>(
      "inside", k_BH1750_addr_vcc_high, "/Sensors/Inside Light Sensor");
  ConfigItem(inside_sensor)
      ->set_title(inside_sensor->get_location() + " light sensor")
      ->set_sort_order(200);

  outside_sensor->begin();
  inside_sensor->begin();

  auto sk_output_outside = std::make_shared<SKOutputIlluminance>("outside");
  ConfigItem(sk_output_outside)
      ->set_title("SK Output for '" + sk_output_outside->get_location() +
                  "' illuminance")
      ->set_sort_order(300);

  auto sk_output_inside = std::make_shared<SKOutputIlluminance>("inside");
  ConfigItem(sk_output_inside)
      ->set_title("SK Output for '" + sk_output_inside->get_location() +
                  "' illuminance")
      ->set_sort_order(400);

  outside_sensor->connect_to(sk_output_outside);
  inside_sensor->connect_to(sk_output_inside);

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
