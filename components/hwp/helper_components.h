
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

#include "PoolHeater.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/button/button.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"

#include <string>

namespace esphome {
namespace hwp {

// 2026 standard: Buttons must explicitly handle the Component lifecycle
class GenerateCodeButton : public button::Button, public Component, public Parented<PoolHeater> {
 public:
  void press_action() override;
  // 2026 Validation: Check parent before use
  void setup() override {
    if (this->parent_ == nullptr) {
      this->mark_failed();
    }
  }
};

// Base class for numeric controls in 2026 to reduce binary size
class PoolHeaterNumber : public number::Number, public Parented<PoolHeater> {
 public:
  PoolHeaterNumber() = default;
 protected:
  void control(float value) override = 0;
};

class d01_defrost_start_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

class d02_defrost_end_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

class d03_defrosting_cycle_time_minutes_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

class d04_max_defrost_time_minutes_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

class d05_min_economy_defrost_time_minutes_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

class r04_return_diff_cooling_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

class r05_shutdown_temp_diff_when_cooling_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

class r06_return_diff_heating_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

class r07_shutdown_diff_heating_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

class u01_flow_meter_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

class u02_pulses_per_liter_number : public PoolHeaterNumber {
 protected:
  void control(float value) override;
};

// Base class for select controls in 2026
class PoolHeaterSelect : public select::Select, public Parented<PoolHeater> {
 public:
  PoolHeaterSelect() = default;
 protected:
  void control(const std::string &value) override = 0;
};

class d06_defrost_eco_mode_select : public PoolHeaterSelect {
 protected:
  void control(const std::string &value) override;
};

class u01_flow_meter_select : public PoolHeaterSelect {
 protected:
  void control(const std::string &value) override;
};

class h02_mode_restrictions_select : public PoolHeaterSelect {
 protected:
  void control(const std::string &value) override;
};

} // namespace hwp
} // namespace esphome
