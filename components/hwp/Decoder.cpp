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
/**
 * @file Decoder.cpp
 * @brief Implements the Decoder class for parsing bit-banged bus frames.
 *
 * MIT License
 * Copyright (c) 2024 S. Leclerc
 *
 * Use at your own risk. No warranty is provided.
 *
 * Compliant with ESPHome 26 / ESP-IDF v6.0 (2026)
 */
#include "Decoder.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include <sstream>

namespace esphome {
namespace hwp {

// ---------------- Frame Control ----------------

void Decoder::reset(const char* msg) {
  if (started_ && msg && strlen(msg) > 0) {
    ESP_LOGV("hwp.decoder", "Decoder reset: %s", msg);
  }
  passes_count_ = 0;
  bit_current_index_ = 0;
  current_byte_value_ = 0;
  started_ = false;
  finalized_ = false;
  source_ = SOURCE_UNKNOWN;
  std::memset(&packet_, 0, sizeof(packet_));
}

void Decoder::start_new_frame() {
  reset("Start of new frame");
  started_ = true;
}

void Decoder::append_bit(bool long_duration) {
  if (!started_) {
    ESP_LOGW("hwp.decoder", "Frame not started. Ignoring bit");
    return;
  }

  if (long_duration) {
    //set_bit(&current_byte_value_, bit_current_index_);
    current_byte_value_ |= (1 << bit_current_index_);

  }

  bit_current_index_++;

  if (bit_current_index_ == 8) {
    if (packet_.data_len < sizeof(packet_.data)) {
      ESP_LOGVV("hwp.decoder", "New byte #%u: 0x%02X", packet_.data_len, current_byte_value_);
      packet_.data[packet_.data_len] = current_byte_value_;
    } else {
      ESP_LOGW("hwp.decoder", "Frame overflow %u/%u. Dropped byte: 0x%02X",
               packet_.data_len, sizeof(packet_.data), current_byte_value_);
    }

    bit_current_index_ = 0;
    current_byte_value_ = 0;
    packet_.data_len++;
  }
}

// ---------------- Validation / Finalization ----------------

std::shared_ptr<BaseFrame> Decoder::finalize(heat_pump_data_t& /*hp_data*/) {
  if (!started_ || packet_.data_len == 0) return nullptr;

  bool inverted = false;
  // Placeholder: implement checksum validation
  // if (!is_checksum_valid(inverted)) return nullptr;

  finalized_ = true;

  auto specialized = std::make_shared<Decoder>(*this);
  specialized->set_frame_time_ms(millis());

  ESP_LOGVV("hwp.decoder", "Frame finalized, type: %s", "DecoderFrame");
  return specialized;
}

bool Decoder::is_valid() const {
  return finalized_;
}

bool Decoder::is_complete() const {
  return started_ && packet_.data_len > 0;
}

void Decoder::is_changed(const BaseFrame& /*frame*/) {
  // Optional: implement frame comparison
}

// ---------------- Status ----------------

bool Decoder::is_started() const { return started_; }
void Decoder::set_started(bool value) { started_ = value; }

void Decoder::debug(const char* msg) {
  if (!msg || packet_.data_len == 0) return;

  std::stringstream oss;
  oss << (started_ ? "STARTED" : "NOT STARTED")
      << ", " << (finalized_ ? "FINALIZED" : "NOT FINALIZED")
      << ", data_len: " << static_cast<int>(packet_.data_len)
      << ", current_byte_value: " << std::hex << static_cast<int>(current_byte_value_)
      << ", bit_index: " << std::dec << static_cast<int>(bit_current_index_);

  ESP_LOGV("hwp.decoder", "%s%s", msg, oss.str().c_str());
}

// ---------------- RMT / Pulse Utils ----------------

int32_t Decoder::get_high_duration(const rmt_symbol_word_t* item) {
  return item->level0 ? static_cast<int32_t>(item->duration0) : 0;
}

uint32_t Decoder::get_low_duration(const rmt_symbol_word_t* item) {
  return !item->level0 ? static_cast<uint32_t>(item->duration0) : 0;
}

bool Decoder::matches_duration(uint32_t target_us, uint32_t actual_us) {
  return actual_us >= (target_us - pulse_duration_threshold_us) &&
         actual_us <= (target_us + pulse_duration_threshold_us);
}

bool Decoder::is_start_frame(const rmt_symbol_word_t* item) {
  return item->level0 && matches_duration(frame_heading_high_duration_ms * 1000, item->duration0);
}

bool Decoder::is_long_bit(const rmt_symbol_word_t* item) {
  return item->level0 && matches_duration(bit_long_high_duration_ms * 1000, item->duration0);
}

bool Decoder::is_short_bit(const rmt_symbol_word_t* item) {
  return item->duration0 > 400 && item->duration0 < 600;
}

bool Decoder::is_frame_end(const rmt_symbol_word_t* item) {
  return item->duration0 == 0 ||
         (!item->level0 && matches_duration(frame_end_threshold_ms * 1000, item->duration0));
}

}  // namespace hwp
}  // namespace esphome





