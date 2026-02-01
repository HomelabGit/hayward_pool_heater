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
#pragma once

#include "BaseFrame.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include <memory>
#include <string>

namespace esphome {
namespace hwp {

// Forward declare heat pump data
struct heat_pump_data_t;

// Simple time structure
struct clock_time_t {
  uint8_t hours = 0;
  uint8_t minutes = 0;
};

// FrameClock class
class FrameClock : public BaseFrame {
 public:
  FrameClock() = default;

  // Frame interface
  std::shared_ptr<BaseFrame> create();
  const char* type_string() const;
  void parse(heat_pump_data_t& hp_data);

  // Formatting
  std::string format_prev() const;
  std::string format(bool no_diff) const;
  std::string format(const clock_time_t& val, const clock_time_t& ref) const;

  // Optional control
  esphome::optional<std::shared_ptr<BaseFrame>> control(const int& call);

  // Matching
  static bool matches(BaseFrame& specialized, BaseFrame& base);

 private:
  clock_time_t data_;
  clock_time_t prev_;
};

}  // namespace hwp
}  // namespace esphome


