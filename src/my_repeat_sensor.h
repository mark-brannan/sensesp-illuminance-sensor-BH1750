#ifndef MY_REPEAT_SENSOR_H_
#define MY_REPEAT_SENSOR_H_

#include "reactesp.h"
#include "sensesp/sensors/sensor.h"

namespace sensesp {
    
template <class T>
class MyRepeatSensor : public Sensor<T> {
 public:

  /**
   * @brief Construct a new RepeatSensor object.
   *
   * RepeatSensor is a sensor that calls a callback function at given intervals
   * and produces the value returned by the callback function. It can be used
   * to wrap any generic Arduino sensor library into a SensESP sensor.
   *
   * @param repeat_interval_ms The repeating interval, in milliseconds.
   * @param callback A callback function that returns the value the sensor will
   * produce.
   * @tparam T The type of the value returned by the callback function.
   */
  MyRepeatSensor(unsigned int repeat_interval_ms, std::function<T()> callback, 
                 const String& config_path = "")
      : Sensor<T>(config_path),
        repeat_interval_ms_(repeat_interval_ms),
        returning_callback_(callback) {
    this->load();
    repeat_event_ = event_loop()->onRepeat(repeat_interval_ms_, [this]() {
      this->emit(this->returning_callback_());
    });
  }

  /**
   * @brief Construct a new RepeatSensor object (supporting asynchronous
   * callbacks).
   *
   * RepeatSensor is a sensor that calls a callback function at given intervals
   * and produces the value returned by the callback function. It can be used
   * to wrap any generic Arduino sensor library into a SensESP sensor.
   *
   * @param repeat_interval_ms The repeating interval, in milliseconds.
   * @param callback A callback function that requires RepeatSensor<T>::emit()
   *   to be called when output becomes available.
   * @tparam T The type of the value returned by the callback function.
   */
  MyRepeatSensor(unsigned int repeat_interval_ms,
               std::function<void(MyRepeatSensor<T>*)> callback,
               const String& config_path = "")
      : Sensor<T>(config_path),
        repeat_interval_ms_(repeat_interval_ms),
        emitting_callback_(callback) {
    this->load();
    repeat_event_ = event_loop()->onRepeat(repeat_interval_ms_,
                           [this]() { emitting_callback_(this); });
  }

  void set_repeat_interval_ms(unsigned int repeat_interval_ms) {
    repeat_interval_ms_ = repeat_interval_ms;
    this->reschedule();
  }

  void reschedule() {
    if (repeat_event_) {
      event_loop()->remove(repeat_event_);
    } else {
      debugE("MyRepeatSensor::reschedule -> repeat_event_ is null");
    }

    if (emitting_callback_) {
      repeat_event_ = event_loop()->onRepeat(
          repeat_interval_ms_, [this]() { emitting_callback_(this); });
    } else if (returning_callback_) {
      repeat_event_ = event_loop()->onRepeat(repeat_interval_ms_, [this]() {
        this->emit(this->returning_callback_());
      });
    } else {
      debugE("MyRepeatSensor::reschedule -> No callback set");
    }
  }

  bool to_json(JsonObject& doc) override {
    doc["repeat_interval_ms"] = repeat_interval_ms_;
    return true;
  }

  bool from_json(const JsonObject& config) override {
    if (config["repeat_interval_ms"].is<unsigned int>()) {
      this->set_repeat_interval_ms(config["repeat_interval_ms"].as<unsigned int>());
    }
    return true;
  }

 protected:
  unsigned int repeat_interval_ms_;
  std::function<T()> returning_callback_ = nullptr;
  std::function<void(MyRepeatSensor<T>*)> emitting_callback_ = nullptr;
  reactesp::RepeatEvent* repeat_event_ = nullptr;
};

template <typename T>
const String ConfigSchema(const MyRepeatSensor<T>& obj) {
  return R"###({"type":"object","properties":{"repeat_interval_ms":{"title":"Repeat interval (ms)","type":"integer"}}})###";
}

}  // namespace sensesp

#endif // MY_REPEAT_SENSOR_H_
