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

#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>
#include <map>
#include <sstream>

#include "Schema.h"
#include "base_frame.h"
#include "Decoder.h"
#include "SpinLockQueue.h"

#include "esphome/core/optional.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/components/logger/logger.h"

#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"

// Enable this line to log bus pulses
#define PULSE_DEBUG

namespace esphome {
namespace hwp {

#ifdef PULSE_DEBUG
static constexpr char TAG_PULSES[] = "hwp.pulses";
#endif

static constexpr char TAG_BUS[] = "hwp";

extern const uint32_t single_frame_max_duration_ms;
extern const uint8_t default_frame_transmit_count;

/**
 * @enum bus_mode_t
 * @brief Bus mode: transmit, receive, or error.
 */
enum class bus_mode_t { TX, RX, ERROR };

/**
 * @class Bus
 * @brief Manages bit-banged bus communication for the pool heater controller.
 *
 * Handles sending and receiving frames using a bit-banging technique, manages queues,
 * tasks, and timing to avoid collisions. Supports optional pulse debugging.
 */
class Bus {
 public:
  // ---- Constructor ----
  explicit Bus(size_t maxWriteLength = 8,
               size_t transmitCount = default_frame_transmit_count);

  // ---- GPIO pin access ----
  void set_gpio_pin(InternalGPIOPin* gpio_pin) { gpio_pin_ = gpio_pin; }
  InternalGPIOPin* get_gpio_pin() const { return gpio_pin_; }

  // ---- Setup ----
  void setup();

  // ---- Queue / send ----
  bool queue_frame_data(std::shared_ptr<BaseFrame> frame);
  void process_send_queue();
  bool has_time_to_send();

  // ---- Controller / timing checks ----
  bool is_controller_timeout() const {
    return previous_controller_packet_time_.has_value() &&
           previous_controller_packet_time_.value() +
                   (delay_between_controller_messages_ms * 1.5) <
               millis();
  }

  bool has_controller() const {
    return controler_packets_received_.has_value() &&
           controler_packets_received_.value();
  }

  esphome::optional<uint32_t> next_controller_packet() const {
    if (previous_controller_packet_time_.has_value()) {
      return previous_controller_packet_time_.value() +
             delay_between_controller_messages_ms;
    }
    if (millis() < delay_between_controller_messages_ms) {
      return delay_between_controller_messages_ms;
    }
    return {};
  }

  bool is_time_for_next() const {
    return !previous_sent_packet_.has_value() ||
           previous_sent_packet_.value() + delay_between_sending_messages_ms <=
               millis();
  }

  bus_mode_t get_bus_mode() const { return mode; }

  void set_data_model(heat_pump_data_t& hp_data) { hp_data_ = &hp_data; }

  // ---- Frame control / traits ----
  std::vector<std::shared_ptr<BaseFrame>> control(const HWPCall& call);
  void traits(climate::ClimateTraits& traits, heat_pump_data_t& hp_data);
  static void dump_known_packets(const char* CALLER_TAG);

 protected:
  heat_pump_data_t* hp_data_{nullptr};
  esphome::optional<bool> controler_packets_received_;
  esphome::optional<uint32_t> previous_controller_packet_time_;
  esphome::optional<uint32_t> previous_sent_packet_;

  volatile bus_mode_t mode{bus_mode_t::RX};
  InternalGPIOPin* gpio_pin_{nullptr};

  TaskHandle_t TxTaskHandle{nullptr};
  TaskHandle_t RxTaskHandle{nullptr};

  Decoder current_frame;
  size_t transmit_count{};
  size_t maxWriteLength{};

  SpinLockQueue<std::shared_ptr<BaseFrame>> received_frames;
  SpinLockQueue<std::shared_ptr<BaseFrame>> tx_packets_queue;

  rmt_channel_handle_t tx_channel_{nullptr};
  rmt_channel_handle_t rx_channel_{nullptr};
  RingbufHandle_t rb_{nullptr};

#ifdef PULSE_DEBUG
  std::vector<std::string> pulse_strings_;
#endif

  uint64_t last_change_us_{0};
  volatile rmt_symbol_word_t current_pulse_;

  // ---- Timing helpers ----
  inline uint64_t elapsed(uint64_t now) const {
    return now >= last_change_us_
               ? now - last_change_us_
               : UINT64_MAX - last_change_us_ + now + 1;
  }

  // ---- Internal methods ----
  void start_receive();
  void process_pulse(rmt_symbol_word_t* item);
  void finalize_frame(bool timeout);
  void store_frame(std::shared_ptr<BaseFrame> frame);

  void sendHeader();
  void sendFrameSpacing();
  void sendGroupSpacing();

  void _sendHigh(uint32_t ms) {
    if (!gpio_pin_) return;
    gpio_pin_->digital_write(true);
    delayMicroseconds(ms * 1000);
  }

  void _sendLow(uint32_t ms) {
    if (!gpio_pin_) return;
    gpio_pin_->digital_write(false);
    delayMicroseconds(ms * 1000);
  }

  // ---- ISR handlers ----
  static void isr_handler(Bus* instance);
  void isr_handler();

  // ---- Tasks ----
  static void TxTask(void* arg);
  static void RxTask(void* arg);

  // ---- Pulse logging ----
  std::string format_pulse_item(const rmt_symbol_word_t* item);
  void log_pulse_item(const rmt_symbol_word_t* item);
  void log_pulses();
  void reset_pulse_log();
};

}  // namespace hwp
}  // namespace esphome

