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

#include "BaseFrame.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include <memory>
#include <cstring>

#ifdef USE_ESP32
#include "driver/rmt_types.h"
#endif


namespace esphome {
namespace hwp {

// Forward declare heat pump data
struct heat_pump_data_t;

// Decoder frame class
class Decoder : public BaseFrame {
 public:
  Decoder() = default;
  Decoder(const Decoder& other) = default;
  Decoder& operator=(const Decoder& other) = default;

  // Frame control
  void reset(const char* msg = nullptr);
  void start_new_frame();
  void append_bit(bool long_duration);

  // Validation / Finalization
  std::shared_ptr<BaseFrame> finalize(heat_pump_data_t& hp_data);
  bool is_valid() const override;
  bool is_complete() const override;
  void is_changed(const BaseFrame& frame);

  // Status
  bool is_started() const;
  void set_started(bool value);
  void debug(const char* msg);

  // RMT / Pulse utilities
  static int32_t get_high_duration(const rmt_symbol_word_t* item);
  static uint32_t get_low_duration(const rmt_symbol_word_t* item);
  static bool matches_duration(uint32_t target_us, uint32_t actual_us);

  // Static pulse checks
  static bool is_start_frame(const rmt_symbol_word_t* item);
  static bool is_long_bit(const rmt_symbol_word_t* item);
  static bool is_short_bit(const rmt_symbol_word_t* item);
  static bool is_frame_end(const rmt_symbol_word_t* item);

 private:
  // Frame state
  bool started_ = false;
  bool finalized_ = false;

  // Bit parsing
  uint8_t current_byte_value_ = 0;
  uint8_t bit_current_index_ = 0;
  uint8_t passes_count_ = 0;

  // Packet storage
  struct {
    uint8_t data[32] = {0};
    size_t data_len = 0;
    uint8_t calculate_checksum() const;
  } packet_;

  // Source info
  enum Source { SOURCE_UNKNOWN, SOURCE_CONTROLLER, SOURCE_HEATER };
  Source source_ = SOURCE_UNKNOWN;

  // Pulse timings
  static constexpr uint32_t pulse_duration_threshold_us = 150;
  static constexpr uint32_t frame_heading_high_duration_ms = 80;
  static constexpr uint32_t bit_long_high_duration_ms = 50;
  static constexpr uint32_t frame_end_threshold_ms = 100;
};

}  // namespace hwp
}  // namespace esphome


