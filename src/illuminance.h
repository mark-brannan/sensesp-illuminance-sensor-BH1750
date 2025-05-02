#ifndef ILLUMINANCE_H
#define ILLUMINANCE_H

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/ui/config_item.h"
#include "sensesp/ui/status_page_item.h"


using namespace sensesp;

class SKOutputIlluminance : public SKOutputFloat {
  public:
   SKOutputIlluminance(const char* location);

   SKOutputIlluminance(
    const char* location,
    const char* sk_path,
    const char* config_path,
    const char* status_page_item_name);

  protected:
   std::shared_ptr<ConfigItemT<SKOutputFloat>> config_item_;
   StatusPageItem<float> status_page_item_;
};

#endif // ILLUMINANCE_H