/**
 *
 * Copyright (c) 2024 S. Leclerc (sle118@hotmail.com)
 *
 * This file is part of the Pool Heater Controller component project.
 *
 * @project Pool Heater Controller Component
 * @developer S. Leclerc (sle118@hotmail.com)
 *
 * @license MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @disclaimer Use at your own risk. The developer assumes no responsibility
 * for any damage or loss caused by the use of this software.
 */
#include "FrameClock.h"
#include "Decoder.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace hwp {

// ---------------- Constructor ----------------

FrameClock::FrameClock() : BaseFrame(), data_(std::make_shared<Decoder>()) {}

// ---------------- Factory ----------------

std::shared_ptr<BaseFrame> FrameClock::create() {
  return std::make_shared<FrameClock>();
}

// ---------------- Type ----------------

const char* FrameClock::type_string() const {
  return "CLOCK";
}

// ---------------- Parse / Decode ----------------

void FrameClock::parse(heat_pump_data_t& hp_data) {
  if (!data_) return;

  // Example: decode hours/minutes from decoder
  hp_data.time.hour = data_->packet_.data_len > 0 ? data_->packet_.data[0] : 0;
  hp_data.time.minute = data_->packet_.data_len > 1 ? data_->packet_.data[1] : 0;

  ESP_LOGVV("hwp.frameclock", "Clock parsed: %02u:%02u", hp_data.time.hour, hp_data.time.minute);
}

// ---------------- Format ----------------

std::string FrameClock::format_prev() const {
  clock_time_t prev{0,0};  // placeholder for previous value
  return format(prev, prev);
}

std::string FrameClock::format(bool no_diff) const {
  clock_time_t val{0,0};
  clock_time_t ref{0,0};
  return format(val, ref);
}

std::string FrameClock::format(const clock_time_t& val, const clock_time_t& ref) const {
  char buf[16];
  snprintf(buf, sizeof(buf), "%02u:%02u", val.hour, val.minute);
  return std::string(buf);
}

// ---------------- Control ----------------

esphome::optional<std::shared_ptr<BaseFrame>> FrameClock::control(const HWPCall& call) {
  // Example placeholder: return this frame if needed
  return std::make_shared<FrameClock>(*this);
}

}  // namespace hwp
}  // namespace esphome

