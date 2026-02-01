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
#include <vector>
#include "base_frame.h"
#include "heat_pump_data.h"

namespace esphome {
namespace hwp {

// Forward declaration
struct rmt_symbol_word_t;

class Decoder : public BaseFrame {
 public:
  // Constructors
  Decoder() = default;
  ~Decoder() override = default;

  // RTTI-free access
  Decoder* as_decoder() override { return this; }

  // Frame processing
  void start_new_frame();
  void append_bit(bool bit);

  // Overrides BaseFrame
  bool is_complete() const override { return finalized_; }
  void traits(climate::ClimateTraits& traits, heat_pump_data_t& hp_data) override {}

  // Static helpers for decoding pulses
  static bool is_start_frame(const rmt_symbol_word_t* item);
  static bool is_long_bit(const rmt_symbol_word_t* item);
  static bool is_short_bit(const rmt_symbol_word_t* item);
  static bool is_frame_end(const rmt_symbol_word_t* item);

 private:
  std::vector<bool> bits_;
  bool started_{false};
  bool finalized_{false};

  // Timing constants (microseconds)
  static constexpr uint32_t frame_heading_high_duration_ms = 4;
  static constexpr uint32_t bit_long_high_duration_ms = 2;
  static constexpr uint32_t frame_end_threshold_ms = 2;

  // Helpers
  static bool matches_duration(uint32_t target_us, uint32_t actual_us);
};

}  // namespace hwp
}  // namespace esphome
