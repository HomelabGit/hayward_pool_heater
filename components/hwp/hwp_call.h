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
#include <optional>

namespace esphome {
namespace hwp {

// Forward declarations of other classes
class DefrostEcoMode;
class FlowMeterEnable;
class HeatPumpRestrict;
class FanMode;
class PoolHeater;

// Alias for heat pump data
using heat_pump_data_t = ::heat_pump_data_t;

/**
 * @brief HWPCall encapsulates a control call to the heat pump via a climate call.
 * Designed for ESPHome 26 compatibility.
 */
class HWPCall {
 public:
  /**
   * @brief Constructor
   * @param call The original climate call object
   * @param parent Reference to the parent PoolHeater
   * @param data Reference to heat pump data struct
   * @param sensor Optional text sensor to report status
   */
  HWPCall(climate::ClimateCall call, PoolHeater &parent, heat_pump_data_t &data,
          text_sensor::TextSensor *sensor = nullptr)
      : call_(call), parent_(parent), data_(data), sensor_(sensor), component(nullptr) {}

  /** Perform the call */
  void perform();

  /** Accessors */
  climate::ClimateCall get_call() const { return call_; }
  heat_pump_data_t &get_data() const { return data_; }

  // Optional values for defrost and return differences
  esphome::optional<float> d01_defrost_start;
  esphome::optional<float> d02_defrost_end;
  esphome::optional<float> d03_defrosting_cycle_time_minutes;
  esphome::optional<float> d04_max_defrost_time_minutes;
  esphome::optional<float> d05_min_economy_defrost_time_minutes;
  esphome::optional<float> r04_return_diff_cooling;
  esphome::optional<float> r05_shutdown_temp_diff_when_cooling;
  esphome::optional<float> r06_return_diff_heating;
  esphome::optional<float> r07_shutdown_diff_heating;
  esphome::optional<float> u02_pulses_per_liter;

  // Optional references to other modules / components
  esphome::optional<DefrostEcoMode*> d06_defrost_eco_mode;
  esphome::optional<FlowMeterEnable*> u01_flow_meter;
  esphome::optional<HeatPumpRestrict*> h02_mode_restrictions;
  esphome::optional<FanMode*> f01_fan_mode;

  /** Optional pointer to the associated component */
  esphome::Component *component;

 protected:
  climate::ClimateCall call_;
  PoolHeater &parent_;
  heat_pump_data_t &data_;
  text_sensor::TextSensor *sensor_;
};

}  // namespace hwp
}  // namespace esphome

