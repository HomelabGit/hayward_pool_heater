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
 * @file Bus.cpp
 * @brief ESPHome 26 compliant bus implementation for Pool Heater Controller
 *
 * MIT License
 * Copyright (c) 2024 S. Leclerc
 *
 * Use at your own risk. No warranty is provided.
 */
/**
 *
 * Copyright (c) 2024 S. Leclerc (sle118@hotmail.com)
 *
 * This file is part of the Pool Heater Controller component project.
 *
 * @project Pool Heater Controller Component
 * @developer S. Leclerc
 *
 * @license MIT License
 *
 * Use at your own risk. No warranty is provided.
 */
#include "Bus.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace hwp {

Bus::Bus(size_t rx_buffer_size, size_t tx_buffer_size)
    : mode_(BUSMODE_RX) {
  rx_frames_.reserve(rx_buffer_size);
  tx_frames_.reserve(tx_buffer_size);
}

void Bus::start_receive() {
  mode_ = BUSMODE_RX;
}

void Bus::process_pulse(rmt_symbol_word_t* item) {
  if (mode_ == BUSMODE_TX || !item) return;

  for (auto& frame : rx_frames_) {
    if (!frame->is_complete()) {
      //auto decoder = std::dynamic_pointer_cast<Decoder>(frame);
      //if (!decoder) continue;
      if (frame->get_type() != FrameType::DECODER)
      continue;
      auto *decoder = static_cast<Decoder *>(frame.get());
        
      if (Decoder::is_start_frame(item))
        decoder->start_new_frame();
      else if (Decoder::is_long_bit(item))
        decoder->append_bit(true);
      else if (Decoder::is_short_bit(item))
        decoder->append_bit(false);
    }
  }
}

void Bus::process_send_queue() {
  if (tx_frames_.empty()) return;

  if (!current_sending_packet_) {
    current_sending_packet_ = tx_frames_.front();
    tx_frames_.erase(tx_frames_.begin());
    ESP_LOGI("hwp.bus", "Sending packet");
    mode_ = BUSMODE_TX;
  }

  sendHeader();
  previous_sent_packet_ = esphome::millis();
  mode_ = BUSMODE_RX;
}

void Bus::sendHeader() {
  ESP_LOGV("hwp.bus", "Sending header: LOW %ums HIGH %ums",
           frame_heading_low_duration_ms, frame_heading_high_duration_ms);
}

bool Bus::is_controller_timeout() const {
  return esphome::millis() > (delay_between_controller_messages_ms * 1.5f);

}

bool Bus::is_time_for_next() const {
  if (!previous_sent_packet_) return true;
  //return millis() >= previous_sent_packet_.value() + delay_between_sending_messages_ms;
  return esphome::millis() >= previous_sent_packet_.value() + delay_between_sending_messages_ms;

}

esphome::optional<unsigned long> Bus::next_controller_packet() const {
  if (tx_frames_.empty()) return {};
  return previous_sent_packet_.value_or(0) + delay_between_controller_messages_ms;
}

//std::vector<std::shared_ptr<BaseFrame>> Bus::control(const HWPCall& call) {
std::vector<std::shared_ptr<BaseFrame>> Bus::control() {

   // std::vector<std::shared_ptr<BaseFrame>> result;
    std::vector<std::shared_ptr<BaseFrame>> result;
    return result;

  for (auto& frame : rx_frames_) {
    if (frame->is_complete()) {
      result.push_back(frame);
    }
  }
  return result;
}

void Bus::traits(climate::ClimateTraits& traits, heat_pump_data_t& hp_data) {
  for (auto& frame : rx_frames_) {
    if (frame->is_complete()) {
      frame->traits(static_cast<void*>(&traits), hp_data);
    }
  }
}

}  // namespace hwp
}  // namespace esphome




