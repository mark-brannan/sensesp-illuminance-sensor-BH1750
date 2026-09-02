#include "illuminance.h"

// These are well-defined paths in the Signal K specification, but a different
// path could be used if you want to do some transformation or remapping in a
// signalk plugin.

// Path for outside illuminance according to the Signal K specification
// https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpenvironmentoutsideilluminance
static const char* kSkPathOutsideIlluminance = "environment.outside.illuminance";

// Path for inside illuminance according to the Signal K specification
// https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpenvironmentinsideilluminance
static const char* kSkPathInsideIlluminance = "environment.inside.illuminance";

static const char* kUIGroup = "Illuminance Sensors";
static const int kUIOrder = 100;

static String get_sk_path_for_location(const char* location) {
  if (strcmp(location, "inside") == 0) {
    return kSkPathInsideIlluminance;
  } else if (strcmp(location, "outside") == 0) {
    return kSkPathOutsideIlluminance;
  }
  return String("environment.") + location + ".illuminance";
}

static String get_sk_out_config_path(const char* location) {
  return String("/config/output/") + location + "/illuminance";
}

static String get_status_page_item_name(const char* location) {
  return String("Value sent to SK from '") + location + "' light sensor";
}

SKOutputIlluminance::SKOutputIlluminance(const char* location)
    : SKOutputIlluminance(location, get_sk_path_for_location(location),
                          get_sk_out_config_path(location),
                          get_status_page_item_name(location)) {}

SKOutputIlluminance::SKOutputIlluminance(const char* location,
                                         const String& sk_path,
                                         const String& config_path,
                                         const String& status_page_item_name)
    : SKOutputFloat(sk_path, config_path, "lux"),
      location_(location),
      status_page_item_(std::make_shared<StatusPageItem<float>>(
          status_page_item_name, -1., kUIGroup, kUIOrder)) {
  this->connect_to(status_page_item_);
}
