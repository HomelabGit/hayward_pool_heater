/**
 *
 * Copyright (c) 2024 S. Leclerc (sle118@hotmail.com)
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

/**
 * @file Decoder.h
 * @brief Decoder class for parsing bit-banged bus frames.
 *
 * MIT License
 * Copyright (c) 2024 S. Leclerc
 *
 * Use at your own risk. No warranty is provided.
 *
 * Compliant with ESPHome 26 / ESP-IDF v6.0 (2026)
 */
#pragma once

#include <memory>
#include <cstring>
#include "base_frame.h"
#include "heat_pump_data.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hwp {

struct rmt_symbol_word_t {
  bool level0;
  uint32_t duration0;
};

// Constants for pulse timings
constexpr uint32_t pulse_duration_threshold_us = 150;
constexpr uint16_t frame_heading_high_duration_ms = 8;
constexpr uint16_t bit_long_high_duration_ms = 2;
constexpr uint16_t frame_end_threshold_ms = 4;

// Example packet struct
struct frame_packet_t {
  uint8_t data[32]{};
  uint8_t data_len{0};

  uint8_t calculate_checksum() const {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < data_len; i++)
      sum += data[i];
    return sum;
  }
};

class Decoder : public BaseFrame {
 public:
  Decoder();
  Decoder(const Decoder& other);
  Decoder& operator=(const Decoder& other);

  // Frame control
  void reset(const char* msg = nullptr);
  void start_new_frame();
  void append_bit(bool long_duration);

  // Finalization / Validation
  std::shared_ptr<BaseFrame> finalize(heat_pump_data_t& hp_data) override;
  bool is_valid() const override;
  bool is_complete() const override;
  void is_changed(const BaseFrame& frame) override;

  // Status
  bool is_started() const;
  void set_started(bool value);
  void debug(const char* msg);

  // Static pulse utils
  static int32_t get_high_duration(const rmt_symbol_word_t* item);
  static uint32_t get_low_duration(const rmt_symbol_word_t* item);
  static bool matches_duration(uint32_t target_us, uint32_t actual_us);
  static bool is_start_frame(const rmt_symbol_word_t* item);
  static bool is_long_bit(const rmt_symbol_word_t* item);
  static bool is_short_bit(const rmt_symbol_word_t* item);
  static bool is_frame_end(const rmt_symbol_word_t* item);

 private:
  frame_packet_t packet_;
  uint8_t current_byte_value_{0};
  uint8_t bit_current_index_{0};
  uint8_t passes_count_{0};
  bool started_{false};
  bool finalized_{false};
};

}  // namespace hwp
}  // namespace esphome

