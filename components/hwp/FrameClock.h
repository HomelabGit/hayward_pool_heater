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
#include "Decoder.h"
#include "esphome/core/helpers.h"
#include <memory>
#include <string>

namespace esphome {
namespace hwp {

struct clock_time_t {
  uint8_t hour;
  uint8_t minute;
};

class FrameClock : public BaseFrame {
 public:
  FrameClock();

  /** Factory method to create a new frame */
  static std::shared_ptr<BaseFrame> create();

  /** Return frame type string */
  const char* type_string() const override;

  /** Parse data into heat pump state */
  void parse(heat_pump_data_t& hp_data) override;

  /** Format helpers */
  std::string format_prev() const override;
  std::string format(bool no_diff) const override;
  std::string format(const clock_time_t& val, const clock_time_t& ref) const;

  /** Control function for specialized frame handling */
  esphome::optional<std::shared_ptr<BaseFrame>> control(const HWPCall& call) override;

 private:
  std::shared_ptr<Decoder> data_;
};

}  // namespace hwp
}  // namespace esphome

