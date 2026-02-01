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

#include "base_frame.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include <vector>
#include "heat_pump_data.h"

// Ensure rmt_symbol_word_t is defined
struct rmt_symbol_word_t {
    uint32_t level0;
    uint32_t duration0;
    uint32_t level1;
    uint32_t duration1;
};

namespace esphome {
namespace hwp {

class Decoder : public BaseFrame {
 public:
  Decoder() = default;

  // RTTI-free cast (optional)
  Decoder* as_decoder() override { return this; }

  // Frame manipulation
  void start_new_frame() {
    started_ = true;
    finalized_ = false;
    bits_.clear();
  }

  void append_bit(bool bit) {
    bits_.push_back(bit);
  }

  bool is_complete() const override { return finalized_; }

  void finalize_frame() { finalized_ = true; }

  // Pulse helpers (RMT symbols)
  static bool is_start_frame(const rmt_symbol_word_t* item);
  static bool is_long_bit(const rmt_symbol_word_t* item);
  static bool is_short_bit(const rmt_symbol_word_t* item);
  static bool is_frame_end(const rmt_symbol_word_t* item);

 private:
  bool started_ = false;
  bool finalized_ = false;
  std::vector<bool> bits_;
};

}  // namespace hwp
}  // namespace esphome




