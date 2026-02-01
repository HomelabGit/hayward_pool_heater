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

#include "Bus.h"
#include "HPUtils.h"
#include "base_frame.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include <memory>

#ifdef USE_ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#ifdef USE_ARDUINO
#include <esp32-hal.h>
#endif
#else
#define millis() (uint32_t)(esp_timer_get_time() / 1000)
#endif

namespace esphome {
namespace hwp {

constexpr uint8_t DEFAULT_FRAME_TRANSMIT_COUNT = 8;

// Max frame duration: longest bits + spacing + heading
constexpr uint32_t SINGLE_FRAME_MAX_DURATION_MS =
    frame_data_length * 8 * (bit_long_high_duration_ms + bit_low_duration_ms) +
    controler_frame_spacing_duration_ms + frame_heading_total_duration_ms;

// -------------------- Bus Class --------------------

Bus::Bus(size_t max_write_length, size_t transmit_count)
    : mode(BUSMODE_RX),
      TxTaskHandle(nullptr),
      RxTaskHandle(nullptr),
      transmit_count(transmit_count),
      maxWriteLength(max_write_length),
      tx_packets_queue(max_write_length) {}

void Bus::setup() {
    current_frame.reset("From setup");
    start_receive();
}

// -------------------- Pulse Processing --------------------

void IRAM_ATTR Bus::process_pulse(rmt_symbol_word_t* item) {
    if (!item) return;

    log_pulse_item(item);

    if (Decoder::is_start_frame(item)) {
        ESP_LOGVV(TAG_BUS, "Start frame detected");

        if (current_frame.is_complete()) {
            finalize_frame(false);
        } else if (current_frame.is_started()) {
            if (!current_frame.is_long_frame() && !current_frame.is_short_frame()) {
                ESP_LOGV(TAG_BUS, "Invalid frame length detected");
            } else if (current_frame.is_size_valid()) {
                current_frame.debug("Starting new frame");
            } else if (!current_frame.is_checksum_valid()) {
                ESP_LOGV(TAG_BUS, "Checksum invalid, trying inverse");
                BaseFrame inv_bf(current_frame);
                inv_bf.inverse();
                ESP_LOGV(TAG_BUS, "Current checksum: %s",
                    current_frame.packet.explain_checksum().c_str());
                ESP_LOGV(TAG_BUS, "Inverted checksum: %s",
                    inv_bf.packet.explain_checksum().c_str());
            }
        }

        reset_pulse_log();
        log_pulse_item(item);
        current_frame.start_new_frame();
    } else if (current_frame.is_started()) {
        bool is_long = Decoder::is_long_bit(item);
        bool is_short = Decoder::is_short_bit(item);

        if (is_long || is_short) {
            current_frame.append_bit(is_long);
        } else if (Decoder::is_frame_end(item)) {
            if (current_frame.is_complete()) {
                finalize_frame(true);
            } else {
                ESP_LOGVV(TAG_BUS, "Bus idle detected (high: %dus, low: %dus)",
                    Decoder::get_high_duration(item), Decoder::get_low_duration(item));
                log_pulses();
            }
        } else if (current_frame.is_complete()) {
            finalize_frame(true);
            reset_pulse_log();
        } else {
            ESP_LOGV(TAG_BUS, "Invalid pulse length (high: %dus, low: %dus)",
                Decoder::get_high_duration(item), Decoder::get_low_duration(item));
            if (current_frame.is_size_valid()) {
                current_frame.debug("Invalid frame - ");
            }
            current_frame.reset("Invalid pulse/frame");
            log_pulses();
        }
    }
}

// -------------------- Queue --------------------

bool Bus::queue_frame_data(std::shared_ptr<BaseFrame> frame) {
    ESP_LOGD(TAG_BUS, "Queueing frame for transmission");
    tx_packets_queue.enqueue(frame);
    return true;
}

// -------------------- Receive --------------------

void Bus::start_receive() {
    current_frame.reset();

    if (!gpio_pin_) {
        ESP_LOGE(TAG_BUS, "Invalid GPIO pin: cannot start receive");
        return;
    }

    ESP_LOGI(TAG_BUS, "Starting reception on pin %d", gpio_pin_->get_pin());
    gpio_pin_->pin_mode(gpio::Flags::FLAG_PULLUP | gpio::Flags::FLAG_INPUT);
    gpio_pin_->attach_interrupt(&Bus::isr_handler, this, gpio::INTERRUPT_ANY_EDGE);

    if (!RxTaskHandle) {
        size_t ring_buf_size = 12 * frame_data_length * (8 + 2) * sizeof(rmt_symbol_word_t);
        rb_ = xRingbufferCreate(ring_buf_size, RINGBUF_TYPE_NOSPLIT);
        ESP_LOGD(TAG_BUS, "Ring buffer created (%u bytes)", ring_buf_size);

        xTaskCreate(RxTask, "RX", 1024 * 11, this, 1, &RxTaskHandle);
        xTaskCreate(TxTask, "TX", 1024 * 15, this, 1, &TxTaskHandle);
    }

    current_frame.reset();
    reset_pulse_log();
    mode = BUSMODE_RX;
    ESP_LOGI(TAG_BUS, "Reception ready on pin %d", gpio_pin_->get_pin());
}

// -------------------- ISR --------------------

void IRAM_ATTR Bus::isr_handler(Bus* instance) { instance->isr_handler(); }

void IRAM_ATTR Bus::isr_handler() {
    if (mode == BUSMODE_TX) return;

    portBASE_TYPE task_awoken = pdFALSE;
    uint64_t now = esp_timer_get_time();
    bool level = gpio_pin_->digital_read();

    if (current_pulse_.duration0 == 0 && level) {
        current_pulse_.level0 = !level;
        current_pulse_.duration0 = elapsed(now);
    } else if (current_pulse_.duration0 > 0) {
        current_pulse_.level1 = !level;
        current_pulse_.duration1 = elapsed(now);

        xRingbufferSendFromISR(rb_, &current_pulse_, sizeof(current_pulse_), &task_awoken);

        memset(&current_pulse_, 0, sizeof(current_pulse_));
    }

    if (task_awoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }

    last_change_us_ = now;
}

// -------------------- Sending --------------------

bool Bus::has_time_to_send() {
    static uint32_t last_send_time = 0;
    bool result = true;

    uint32_t current_time = millis();
    uint32_t end_transmit = current_time + transmit_count * SINGLE_FRAME_MAX_DURATION_MS;

    if (has_controller() && is_controller_timeout()) {
        return true;
    }

    if (next_controller_packet().has_value()) {
        result = end_transmit < next_controller_packet().value();
    }

    if (current_time - last_send_time < 1000) {
        ESP_LOGV(TAG_BUS, "Throttling send. Result=%s", result ? "true" : "false");
        last_send_time = current_time;
    }

    return result;
}

// -------------------- Send Queue --------------------

void Bus::process_send_queue() {
    if (!tx_packets_queue.has_next()) return;
    tx_packets_queue.logging_enabled = true;

    if (current_frame.is_started() || !is_time_for_next() || !has_time_to_send()) {
        return;
    }

    std::shared_ptr<BaseFrame> packet;
    if (!tx_packets_queue.try_dequeue(&packet)) return;

    ESP_LOGI(TAG_BUS, "Sending packet type: %s", packet->type_string());
    mode = BUSMODE_TX;
    current_frame.reset("TX Start");
    reset_pulse_log();
    packet->print("SEND", TAG_BUS, ESPHOME_LOG_LEVEL_INFO, __LINE__);

    gpio_pin_->pin_mode(gpio::Flags::FLAG_OUTPUT | gpio::Flags::FLAG_PULLUP);
    size_t count = transmit_count;

    while (count-- > 0) {
        sendHeader();
        for (size_t txIndex = 0; txIndex < packet->size(); ++txIndex) {
            for (int bitIndex = 0; bitIndex < 8; ++bitIndex) {
                _sendLow(bit_low_duration_ms);
                if (get_bit((*packet)[txIndex], bitIndex)) {
                    _sendHigh(bit_long_high_duration_ms);
                } else {
                    _sendHigh(bit_low_duration_ms);
                }
            }
        }

        if (count > 0) {
            _sendLow(bit_low_duration_ms);
            _sendHigh(controler_frame_spacing_duration_ms);
        }
    }

    _sendLow(bit_low_duration_ms);
    _sendHigh(controler_group_spacing_ms);

    previous_sent_packet_ = millis();
    start_receive();
}

// -------------------- Finalizing Frame --------------------

void IRAM_ATTR Bus::finalize_frame(bool timeout) {
    auto finalized_frame = current_frame.finalize(*hp_data_);
    if (!finalized_frame) {
        if (timeout) {
            current_frame.reset("Timeout - ");
            log_pulses();
        }
        return;
    }

    ESP_LOGVV(TAG_BUS, "Frame finalized%s", timeout ? " (timeout)" : "");

    if (finalized_frame->get_source() == SOURCE_CONTROLLER) {
        controler_packets_received_ = true;
        previous_controller_packet_time_ = millis();
    }

    current_frame.reset();
    reset_pulse_log();
}

// -------------------- Utility --------------------

void Bus::sendHeader() {
    if (!gpio_pin_) return;
    _sendLow(frame_heading_low_duration_ms);
    _sendHigh(frame_heading_high_duration_ms);
}

std::vector<std::shared_ptr<BaseFrame>> Bus::control(const HWPCall& call) {
    std::vector<std::shared_ptr<BaseFrame>> result;
    auto& registry = BaseFrame::get_registry();

    for (auto& entry : registry) {
        auto frame_opt = entry.instance->control(call);
        if (frame_opt.has_value()) {
            frame_opt.value()->print("PUSH", TAG_BUS, ESPHOME_LOG_LEVEL_VERBOSE, __LINE__);
            result.push_back(frame_opt.value());
        }
    }
    return result;
}

void Bus::traits(climate::ClimateTraits& traits, heat_pump_data_t& hp_data) {
    for (auto& entry : BaseFrame::get_registry()) {
        entry.instance->traits(traits, hp_data);
    }
}

}  // namespace hwp
}  // namespace esphome

