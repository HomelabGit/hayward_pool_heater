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
#include "esphome/core/log.h"

namespace esphome {
namespace hwp {

// ---------------- Create / Type ----------------

std::shared_ptr<BaseFrame> FrameClock::create() {
  return std::make_shared<FrameClock>();
}

const char* FrameClock::type_string() const {
  return "CLOCK";
}

// ---------------- Parse / Control ----------------

void FrameClock::parse(heat_pump_data_t& hp_data) {
  hp_data.time = data_;  // assign current clock data
}

esphome::optional<std::shared_ptr<BaseFrame>> FrameClock::control(const int& /*call*/) {
  return {};  // no control logic implemented
}

// ---------------- Formatting ----------------

std::string FrameClock::format_prev() const {
  return format(prev_, data_);
}

std::string FrameClock::format(bool no_diff) const {
  if (no_diff)
    return format(data_, data_);
  return format(prev_, data_);
}

std::string FrameClock::format(const clock_time_t& val, const clock_time_t& ref) const {
  char buf[16];
  snprintf(buf, sizeof(buf), "%02u:%02u", val.hours, val.minutes);
  return std::string(buf);
}

// ---------------- Matching ----------------

bool FrameClock::matches(BaseFrame& specialized, BaseFrame& base) {
  auto* a = dynamic_cast<FrameClock*>(&specialized);
  auto* b = dynamic_cast<FrameClock*>(&base);
  if (!a || !b) return false;

  return a->data_.hours == b->data_.hours &&
         a->data_.minutes == b->data_.minutes;
}

}  // namespace hwp
}  // namespace esphome


