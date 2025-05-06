#include "sensesp/signalk/signalk_output.h"
#include "sensesp/ui/config_item.h"
#include "sensesp/ui/status_page_item.h"
#include "illuminance.h"

// These are well-defined paths in the Signal K specification, but a different path could be used
// if you want to do some transformation or remapping in a signalk plugin.

// Path for output illuminance according to the Signal K specification
// https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpenvironmentinsideilluminance
const char* sk_path_outside_illuminance = "environment.outside.illuminance";

// Path for inside illuminance according to the Signal K specification
// https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpenvironmentoutsideilluminance
const char* sk_path_inside_illuminance = "environment.inside.illuminance";

const char* kUIGroup = "Illuminance Sensors";
const int kUIOrder = 100;


inline const char* get_sk_path_for_location(const char* location) {
    if (strcmp(location, "inside") == 0) {
        return sk_path_inside_illuminance;
    } else if (strcmp(location, "outside") == 0) {
        return sk_path_outside_illuminance;
    } else {
        auto sk_path = String("environment." + String(location) + ".illuminance");
        return sk_path.c_str();
    }
};

inline const char* get_sk_out_config_path(const char* location) {
    if (strcmp(location, "inside") == 0) {
        return "/config/output/inside/illuminance";
    } else if (strcmp(location, "outside") == 0) {
        return "/config/output/outside/illuminance";
    } else {
        auto config_path = String("/config/output/" + String(location) + "/illuminance");
        return config_path.c_str();
    }
};


inline const char* get_status_page_item_name(const char* location) {
    if (strcmp(location, "inside") == 0) {
        return "Value sent to SK from 'inside' light sensor";
    } else if (strcmp(location, "outside") == 0) {
        return "Value sent to SK from 'outside' light sensor";
    } else {
        return String("Value sent to SK from '" + String(location) + "' light sensor").c_str();
    }
};


 SKOutputIlluminance::SKOutputIlluminance(const char* location)
 : SKOutputIlluminance(
    location,
    get_sk_path_for_location(location),
    get_sk_out_config_path(location),
    get_status_page_item_name(location)
) {};

SKOutputIlluminance::SKOutputIlluminance(
    const char* location,
    const char* sk_path,
    const char* config_path,
    const char* status_page_name
) : SKOutputFloat(sk_path, config_path, "lux"),
    status_page_item_(status_page_name, -1., kUIGroup, kUIOrder1) {
    ConfigItem(this)->set_title("SK Output for '" + String(location) + "' illuminance");
    this->connect_to(status_page_item_);
};
