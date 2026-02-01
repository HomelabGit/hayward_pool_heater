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
#include <cstdint>
#include <memory>
#include <cstring>
#include "base_frame.h"

namespace esphome {
namespace hwp {

class Decoder : public BaseFrame {
public:
    Decoder() = default;
    ~Decoder() override = default;

    void start_new_frame() override {
        data_len_ = 0;
        finalized_ = false;
        current_byte_ = 0;
        bit_index_ = 0;
    }

    void append_bit(bool long_bit) override {
        if (bit_index_ == 8) {
            if (data_len_ < sizeof(data_))
                data_[data_len_++] = current_byte_;
            current_byte_ = 0;
            bit_index_ = 0;
        }
        current_byte_ <<= 1;
        if (long_bit)
            current_byte_ |= 1;
        ++bit_index_;
    }

    std::shared_ptr<BaseFrame> finalize(heat_pump_data_t& /*hp_data*/) override {
        if (bit_index_ > 0 && data_len_ < sizeof(data_))
            data_[data_len_++] = current_byte_;
        finalized_ = true;
        return std::make_shared<Decoder>(*this);
    }

    bool is_complete() const override { return finalized_; }

    // Timing constants
    inline static constexpr uint32_t bit_long_high_duration_ms = 800;
    inline static constexpr uint32_t bit_low_duration_ms = 500;
    inline static constexpr uint32_t frame_heading_high_duration_ms = 1500;
    inline static constexpr uint32_t frame_heading_low_duration_ms = 700;

    static bool is_start_frame(const rmt_symbol_word_t* item) {
        return item->level0 && item->duration0 >= frame_heading_high_duration_ms * 1000;
    }
    static bool is_long_bit(const rmt_symbol_word_t* item) {
        return item->level0 && item->duration0 >= bit_long_high_duration_ms * 1000;
    }
    static bool is_short_bit(const rmt_symbol_word_t* item) {
        return item->level0 && item->duration0 >= bit_low_duration_ms * 1000 &&
               item->duration0 < bit_long_high_duration_ms * 1000;
    }
    static bool is_frame_end(const rmt_symbol_word_t* item) {
        return !item->level0;
    }

private:
    uint8_t current_byte_{0};
    uint8_t bit_index_{0};
};

}  // namespace hwp
}  // namespace esphome



