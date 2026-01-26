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

/**  C++ header compliant with ESPHome 2026.1.0 */ 

#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace hwp {

// Ensure your enums/structs are defined or included before this class
struct heat_pump_data_t; 
enum class DefrostEcoMode : uint8_t;
enum class FlowMeterEnable : uint8_t;
enum class HeatPumpRestrict : uint8_t;
enum class FanMode : uint8_t;

class HWPCall : public climate::ClimateCall {
 public:
  // 2026 compliant constructor using member initializer list
  HWPCall(climate::Climate *parent, Component *component, heat_pump_data_t &hp_data,
          text_sensor::TextSensor *status)
      : climate::ClimateCall(parent), component_(component), hp_data_(hp_data), status_(status) {}

  // Added override for 2026 Climate API standardization
  void perform() override; 

  // Use protected/private members with getters for better 2026 stability
  climate::Climate &get_parent() { return *this->parent_; }

  // 2026 standard uses pointers for cross-component references to support ESP-IDF
  esphome::Component *component_;
  heat_pump_data_t &hp_data_;
  text_sensor::TextSensor *status_;

  // Optional parameters remain standard
  optional<float> d01_defrost_start;
  optional<float> d02_defrost_end;
  optional<float> d03_defrosting_cycle_time_minutes;
  optional<float> d04_max_defrost_time_minutes;
  optional<float> d05_min_economy_defrost_time_minutes;
  optional<float> r04_return_diff_cooling;
  optional<float> r05_shutdown_temp_diff_when_cooling;
  optional<float> r06_return_diff_heating;
  optional<float> r07_shutdown_diff_heating;
  optional<float> u02_pulses_per_liter;
  optional<DefrostEcoMode> d06_defrost_eco_mode;
  optional<FlowMeterEnable> u01_flow_meter;
  optional<HeatPumpRestrict> h02_mode_restrictions;
  optional<FanMode> f01_fan_mode;
};

}  // namespace hwp
}  // namespace esphome
