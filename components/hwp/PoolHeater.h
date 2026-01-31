/**
 * @file PoolHeater.h
 * @brief Defines the PoolHeater class for handling communication with the heat pump.
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

#include "Bus.h"
#include "HeaterStatus.h"
#include "Schema.h"
#include "base_frame.h"
#include "hwp_call.h"

#include "esphome/components/climate/climate.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "esphome/core/application.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/macros.h"

namespace esphome {
namespace hwp {

namespace sensor = esphome::sensor;
namespace binary_sensor = esphome::binary_sensor;

extern const char *POOL_HEATER_TAG;

static constexpr uint8_t POOLHEATER_TEMP_MIN = 15;
static constexpr uint8_t POOLHEATER_TEMP_MAX = 33;

class PoolHeater : public climate::Climate, public PollingComponent {
 public:
  explicit PoolHeater(InternalGPIOPin *gpio_pin);

  /* ---------- ESPHome lifecycle ---------- */
  void setup() override;
  void update() override;
  void dump_config() override;

  /* ---------- Climate ---------- */
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  /* ---------- Custom control ---------- */
  void control(const HWPCall &call);
  void generate_code();

  /* ---------- Mode control ---------- */
  void set_passive_mode(bool passive);
  void set_update_active(bool active);
  bool get_passive_mode() const;
  bool is_update_active() const;

  /* ---------- Data access ---------- */
  heat_pump_data_t &data() { return hp_data_; }

  /* ---------- Text sensors ---------- */
  void set_actual_status_sensor(text_sensor::TextSensor *sensor);
  void set_heater_status_code_sensor(text_sensor::TextSensor *sensor);
  void set_heater_status_description_sensor(text_sensor::TextSensor *sensor);
  void set_heater_status_solution_sensor(text_sensor::TextSensor *sensor);

  /* ---------- Temperature sensors ---------- */
  void set_suction_temperature_T01_sensor(sensor::Sensor *sensor);
  void set_outlet_temperature_T03_sensor(sensor::Sensor *sensor);
  void set_coil_temperature_T04_sensor(sensor::Sensor *sensor);
  void set_ambient_temperature_T05_sensor(sensor::Sensor *sensor);
  void set_exhaust_temperature_T06_sensor(sensor::Sensor *sensor);

  /* ---------- Numbers ---------- */
  void set_d01_defrost_start_sensor(number::Number *sensor);
  void set_d02_defrost_end_sensor(number::Number *sensor);
  void set_d03_defrosting_cycle_time_minutes_sensor(number::Number *sensor);
  void set_d04_max_defrost_time_minutes_sensor(number::Number *sensor);
  void set_d05_min_economy_defrost_time_minutes_sensor(number::Number *sensor);
  void set_r04_return_diff_cooling_sensor(number::Number *sensor);
  void set_r05_shutdown_temp_diff_when_cooling_sensor(number::Number *sensor);
  void set_r06_return_diff_heating_sensor(number::Number *sensor);
  void set_r07_shutdown_diff_heating_sensor(number::Number *sensor);
  void set_u02_pulses_per_liter_sensor(number::Number *sensor);

  /* ---------- Binary / select ---------- */
  void set_s02_water_flow_sensor(binary_sensor::BinarySensor *sensor);
  void set_h02_mode_restrictions_sensor(select::Select *sensor);
  void set_u01_flow_meter_sensor(select::Select *sensor);
  void set_d06_defrost_eco_mode_sensor(select::Select *sensor);

  /* ---------- Setpoints ---------- */
  void set_r01_setpoint_cooling_sensor(sensor::Sensor *sensor);
  void set_r02_setpoint_heating_sensor(sensor::Sensor *sensor);
  void set_r03_setpoint_auto_sensor(sensor::Sensor *sensor);
  void set_r08_min_cool_setpoint_sensor(sensor::Sensor *sensor);
  void set_r09_max_cooling_setpoint_sensor(sensor::Sensor *sensor);
  void set_r10_min_heating_setpoint_sensor(sensor::Sensor *sensor);
  void set_r11_max_heating_setpoint_sensor(sensor::Sensor *sensor);

 protected:
  /* ---------- Internal state ---------- */
  heat_pump_data_t hp_data_;
  Bus driver_;
  HeaterStatus heater_status_;
  std::string actual_status_;

  bool passive_mode_{true};
  bool update_active_{false};

  /* ---------- Text sensors ---------- */
  text_sensor::TextSensor *actual_status_sensor{nullptr};
  text_sensor::TextSensor *heater_status_code_sensor_{nullptr};
  text_sensor::TextSensor *heater_status_description_sensor_{nullptr};
  text_sensor::TextSensor *heater_status_solution_sensor_{nullptr};

  /* ---------- Sensors ---------- */
  sensor::Sensor *t01_temperature_suction_sensor{nullptr};
  sensor::Sensor *t03_temperature_outlet_sensor{nullptr};
  sensor::Sensor *t04_temperature_coil_sensor{nullptr};
  sensor::Sensor *t05_temperature_ambient_sensor{nullptr};
  sensor::Sensor *t06_temperature_exhaust_sensor{nullptr};

  sensor::Sensor *r01_setpoint_cooling_sensor{nullptr};
  sensor::Sensor *r02_setpoint_heating_sensor{nullptr};
  sensor::Sensor *r03_setpoint_auto_sensor{nullptr};
  sensor::Sensor *r08_min_cool_setpoint_sensor{nullptr};
  sensor::Sensor *r09_max_cooling_setpoint_sensor{nullptr};
  sensor::Sensor *r10_min_heating_setpoint_sensor{nullptr};
  sensor::Sensor *r11_max_heating_setpoint_sensor{nullptr};

  number::Number *d01_defrost_start_sensor{nullptr};
  number::Number *d02_defrost_end_sensor{nullptr};
  number::Number *d03_defrosting_cycle_time_minutes_sensor{nullptr};
  number::Number *d04_max_defrost_time_minutes_sensor{nullptr};
  number::Number *d05_min_economy_defrost_time_minutes_sensor{nullptr};

  number::Number *r04_return_diff_cooling_sensor{nullptr};
  number::Number *r05_shutdown_temp_diff_when_cooling_sensor{nullptr};
  number::Number *r06_return_diff_heating_sensor{nullptr};
  number::Number *r07_shutdown_diff_heating_sensor{nullptr};
  number::Number *u02_pulses_per_liter_sensor{nullptr};

  binary_sensor::BinarySensor *s02_water_flow_sensor{nullptr};
  select::Select *h02_mode_restrictions_sensor{nullptr};
  select::Select *u01_flow_meter_sensor{nullptr};
  select::Select *d06_defrost_eco_mode_sensor{nullptr};

  /* ---------- Preferences ---------- */
  ESPPreferenceObject preferences_;
  struct PoolHeaterPreferences {
    bool dummy{false};
  };

  /* ---------- Helpers ---------- */
  bool is_heater_offline() const {
    return (!hp_data_.last_heater_frame.has_value() ||
            (millis() - hp_data_.last_heater_frame.value()) > 30000);
  }

  void set_actual_status(const std::string &status, bool force = false);
  void restore_preferences_();
  void save_preferences_();

  /* ---------- Publishing helpers ---------- */
  template<typename T, typename U>
  void publish_sensor_value(const optional<U> &value, T *sensor) {
    if (!sensor || !update_active_ || !value.has_value())
      return;
    sensor->publish_state(value.value());
  }

  template<typename T>
  void publish_sensor_value(const optional<T> &value, select::Select *sensor) {
    if (!sensor || !update_active_ || !value.has_value())
      return;
    sensor->publish_state(value.value().to_string());
  }

  template<typename T>
  void publish_sensor_value(const T &value, T *sensor) {
    if (!sensor || !update_active_)
      return;
    sensor->publish_state(value);
  }
};

}  // namespace hwp
}  // namespace esphome

