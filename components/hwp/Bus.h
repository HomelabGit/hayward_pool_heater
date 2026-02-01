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

#include "BaseFrame.h"
#include "Decoder.h"
#include "esphome/components/climate/climate.h"
#include "esphome/core/helpers.h"
#include <vector>
#include <memory>
#include <optional>

namespace esphome {
namespace hwp {

enum BusMode { BUSMODE_RX, BUSMODE_TX };

class Bus {
 public:
  Bus(size_t rx_buffer_size = 16, size_t tx_buffer_size = 16);

  /** Start receiving mode */
  void start_receive();

  /** Process incoming pulse from RMT */
  void process_pulse(rmt_symbol_word_t* item);

  /** Send queued frames */
  void process_send_queue();

  /** Check if controller timed out */
  bool is_controller_timeout() const;

  /** Check if it's time for next send */
  bool is_time_for_next() const;

  /** Return timestamp for next controller packet */
  esphome::optional<unsigned long> next_controller_packet() const;

  /** Access received frames */
  std::vector<std::shared_ptr<BaseFrame>> control(const HWPCall& call);

  /** Update climate traits based on frames */
  void traits(climate::ClimateTraits& traits, heat_pump_data_t& hp_data);

 private:
  void sendHeader();

  BusMode mode_;
  std::vector<std::shared_ptr<BaseFrame>> rx_frames_;
  std::vector<std::shared_ptr<BaseFrame>> tx_frames_;
  std::shared_ptr<BaseFrame> current_sending_packet_;
  esphome::optional<unsigned long> previous_sent_packet_;

  // Timing configuration
  const unsigned long delay_between_sending_messages_ms = 100;
  const unsigned long delay_between_controller_messages_ms = 1000;
  const unsigned long frame_heading_low_duration_ms = 8;
  const unsigned long frame_heading_high_duration_ms = 8;
};

}  // namespace hwp
}  // namespace esphome





