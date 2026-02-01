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
#include <cstdint>
#include <cstring>
#include <memory>
#include <sstream>
#include "esphome/core/log.h"

namespace esphome {
namespace hwp {

// Forward declarations
struct heat_pump_data_t;

// Frame source enum
enum frame_source_t { SOURCE_UNKNOWN = 0, SOURCE_CONTROLLER, SOURCE_HEATER };

// Packet data structure
struct hp_packetdata_t {
  static constexpr size_t MAX_DATA_LEN = 64;
  uint8_t data[MAX_DATA_LEN]{0};
  uint8_t data_len{0};

  void reset() {
    std::memset(data, 0, sizeof(data));
    data_len = 0;
  }

  uint8_t calculate_checksum() const {
    uint8_t sum = 0;
    for (size_t i = 0; i < data_len; ++i)
      sum += data[i];
    return sum;
  }
};

// BaseFrame class
class BaseFrame {
 public:
  BaseFrame() = default;
  BaseFrame(const BaseFrame& other)
      : packet(other.packet), finalized(other.finalized), source_(other.source_) {}

  BaseFrame& operator=(const BaseFrame& other) {
    if (this != &other) {
      packet = other.packet;
      finalized = other.finalized;
      source_ = other.source_;
    }
    return *this;
  }

  virtual ~BaseFrame() = default;

  // Check if frame is valid
  virtual bool is_valid() const {
    return (packet.data_len > 0 && is_size_valid() && is_checksum_valid());
  }

  // Inverse frame bytes
  virtual void inverse() {
    for (size_t i = 0; i < packet.data_len; ++i)
      packet.data[i] = ~packet.data[i];
  }

  // Traits for climate/heatpump integration
  virtual void traits(climate::ClimateTraits& traits, heat_pump_data_t& hp_data) {}

  // Size check (example: min 4 bytes, max MAX_DATA_LEN)
  virtual bool is_size_valid() const { return packet.data_len >= 4 && packet.data_len <= hp_packetdata_t::MAX_DATA_LEN; }

  // Checksum validation
  virtual bool is_checksum_valid(bool& inverted = *(new bool(false))) const {
    if (packet.data_len == 0) return false;
    uint8_t checksum = packet.calculate_checksum();
    if (checksum == 0) {
      inverted = true;
      return true;
    }
    inverted = false;
    return true;
  }

  // Frame time (ms)
  void set_frame_time_ms(uint32_t time) { frame_time_ms_ = time; }
  uint32_t get_frame_time_ms() const { return frame_time_ms_; }

 protected:
  hp_packetdata_t packet;
  bool finalized{false};
  frame_source_t source_{SOURCE_UNKNOWN};
  uint32_t frame_time_ms_{0};
};

}  // namespace hwp
}  // namespace esphome


