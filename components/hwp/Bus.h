/**
 * @file Bus.h
 * @brief Defines the Bus class for handling bus communication using bit-banging technique.
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
 * @file Bus.h
 * @brief Defines the Bus class for handling bit-banged bus communication.
 *
 * MIT License
 * Copyright (c) 2024 S. Leclerc
 *
 * Use at your own risk. No warranty is provided.
 */
#pragma once
#include <vector>
#include <memory>
#include "esphome/core/log.h"
#include "esphome/core/optional.h"
#include "Decoder.h"
#include "base_frame.h"

// Forward declaration of climate traits to avoid full include
namespace esphome {
namespace climate {
class ClimateTraits;
}  // namespace climate
}  // namespace esphome

namespace esphome {
namespace hwp {

// Forward declaration of heat pump control call
struct HWPCall { 
  int command; 
};

enum BusMode { BUSMODE_RX, BUSMODE_TX };

class Bus {
 public:
  Bus(size_t rx_buffer_size = 16, size_t tx_buffer_size = 16);

  void start_receive();
  void process_pulse(rmt_symbol_word_t* item);
  void process_send_queue();
  void sendHeader();
  
  bool is_controller_timeout() const;
  bool is_time_for_next() const;
  esphome::optional<unsigned long> next_controller_packet() const;
  
  std::vector<std::shared_ptr<BaseFrame>> control(const HWPCall& call);

  void traits(esphome::climate::ClimateTraits& traits, heat_pump_data_t& hp_data);

 private:
  BusMode mode;

  // Timing constants
  inline static constexpr uint32_t delay_between_controller_messages_ms = 1000;
  inline static constexpr uint32_t delay_between_sending_messages_ms = 150;
  inline static constexpr uint32_t bit_long_high_duration_ms = Decoder::bit_long_high_duration_ms;
  inline static constexpr uint32_t bit_low_duration_ms = Decoder::bit_low_duration_ms;
  inline static constexpr uint32_t frame_heading_high_duration_ms = Decoder::frame_heading_high_duration_ms;
  inline static constexpr uint32_t frame_heading_low_duration_ms = 500;
  inline static constexpr uint32_t controler_frame_spacing_duration_ms = 200;
  inline static constexpr uint32_t controler_group_spacing_ms = 400;
  
  std::vector<std::shared_ptr<BaseFrame>> rx_frames_;
  std::vector<std::shared_ptr<BaseFrame>> tx_frames_;
  std::shared_ptr<BaseFrame> current_sending_packet_;
  esphome::optional<unsigned long> previous_sent_packet_;
};

}  // namespace hwp
}  // namespace esphome



