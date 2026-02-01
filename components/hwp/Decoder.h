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

#include <cstring>
#include <memory>
#include "esphome/core/log.h"
#include "base_frame.h"
#include "driver/rmt_rx.h"  // Mandatory for rmt_symbol_word_t in 2026

namespace esphome {
namespace hwp {

static constexpr char TAG_DECODING[] = "hwp.decoding";

class Decoder : public BaseFrame {
 public:
  // --- Constructors & assignment ---
  Decoder();
  Decoder(const Decoder& other);
  Decoder& operator=(const Decoder& other);

  // --- Frame lifecycle ---
  void reset(const char* msg = "");
  void start_new_frame();
  std::shared_ptr<BaseFrame> finalize(heat_pump_data_t& hp_data);
  
  // --- Bit operations ---
  void append_bit(bool long_duration);

  // --- Status checks ---
  bool is_valid() const;
  bool is_complete() const;
  bool is_started() const;
  void set_started(bool value);
  void is_changed(const BaseFrame& frame);  // placeholder for frame change detection
  bool is_finalized() const { return finalized; }

  // --- Debugging ---
  void debug(const char* msg = "");

  // --- Pulse helpers (2026 compliant) ---
  static int32_t get_high_duration(const rmt_symbol_word_t* item);
  static uint32_t get_low_duration(const rmt_symbol_word_t* item);
  static bool matches_duration(uint32_t target_us, uint32_t actual_us);
  static bool is_start_frame(const rmt_symbol_word_t* item);
  static bool is_long_bit(const rmt_symbol_word_t* item);
  static bool is_short_bit(const rmt_symbol_word_t* item);
  static bool is_frame_end(const rmt_symbol_word_t* item);

  // --- Counters ---
  uint32_t passes_count{0};

 private:
  uint8_t current_byte_value{0};
  uint8_t bit_current_index{0};
  bool started{false};
};

}  // namespace hwp
}  // namespace esphome

