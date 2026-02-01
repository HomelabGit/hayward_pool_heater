#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include <memory>

namespace esphome {
namespace hwp {

// Forward declare heat pump data
struct heat_pump_data_t;
class Decoder;  // forward declaration

// Base class for all frames
class BaseFrame {
 public:
  BaseFrame() = default;
  virtual ~BaseFrame() = default;

  enum class FrameType {
    BASE,
    DECODER,
  };
 
virtual FrameType get_type() const { return FrameType::BASE; }

  // Must implement to check if frame is valid
  virtual bool is_complete() const = 0;
  virtual bool is_valid() const { return true; }

  // Optional: parse frame into climate traits
  virtual void traits(void* traits, heat_pump_data_t& hp_data) {}

  // Optional: set timestamp for frame
  void set_frame_time_ms(unsigned long time) { frame_time_ms_ = time; }
  unsigned long frame_time_ms() const { return frame_time_ms_; }

 private:
  unsigned long frame_time_ms_ = 0;

 

};

}  // namespace hwp
}  // namespace esphome
