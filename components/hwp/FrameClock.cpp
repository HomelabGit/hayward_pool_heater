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

namespace esphome {
namespace hwp {

CLASS_ID_DECLARATION(esphome::hwp::FrameClock)

std::shared_ptr<BaseFrame> FrameClock::create() { return std::make_shared<FrameClock>(); }

const char *FrameClock::type_string() const { return "CLOCK"; }

bool FrameClock::matches(BaseFrame & /*specialized*/, BaseFrame &base) {
  return base.packet.get_type() == FRAME_ID_CLOCK;
}

void FrameClock::parse(heat_pump_data_t &hp_data) {
  if (this->data_.has_value()) {
    hp_data.time = this->data_.value().decode();
  }
}

std::string FrameClock::format_prev() const {
  if (!this->prev_data_.has_value())
    return "N/A";
  return this->format(this->prev_data_.value(), this->prev_data_.value());
}

std::string FrameClock::format(bool no_diff) const {
  if (!this->data_.has_value())
    return "N/A";
  const auto &cur = this->data_.value();
  const auto ref = (no_diff ? cur : this->prev_data_.value_or(cur));
  return this->format(cur, ref);
}

std::string FrameClock::format(const clock_time_t &val, const clock_time_t &ref) const {
  return val.diff(ref);
}

esphome::optional<std::shared_ptr<BaseFrame>> FrameClock::control(const HWPCall & /*call*/) {
  return esphome::nullopt;
}

}  // namespace hwp
}  // namespace esphome
