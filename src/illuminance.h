#ifndef ILLUMINANCE_H
#define ILLUMINANCE_H

#include <memory>

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/ui/status_page_item.h"

using namespace sensesp;

/**
 * @brief SKOutputFloat for an illuminance reading at a named location, with a
 * status page item showing the last value sent.
 *
 * For "inside" and "outside" the Signal K path is the well-known one from the
 * specification; any other location maps to
 * "environment.<location>.illuminance". The config path is
 * "/config/output/<location>/illuminance".
 *
 * Register with ConfigItem(output) after construction to expose the SK path in
 * the web UI.
 */
class SKOutputIlluminance : public SKOutputFloat {
 public:
  SKOutputIlluminance(const char* location);

  SKOutputIlluminance(const char* location, const String& sk_path,
                      const String& config_path,
                      const String& status_page_item_name);

  const String& get_location() const { return location_; }

 protected:
  String location_;
  std::shared_ptr<StatusPageItem<float>> status_page_item_;
};

#endif  // ILLUMINANCE_H
