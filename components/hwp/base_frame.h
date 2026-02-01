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
#include <memory>
#include "esphome/core/log.h"
#include "driver/rmt_rx.h"

namespace esphome {
namespace hwp {

// Forward declaration
struct heat_pump_data_t;

// Base frame class
class BaseFrame {
public:
    BaseFrame() = default;
    virtual ~BaseFrame() = default;

    virtual void start_new_frame() {}
    virtual void append_bit(bool long_bit) {}
    virtual std::shared_ptr<BaseFrame> finalize(heat_pump_data_t& hp_data) { return nullptr; }
    virtual bool is_complete() const { return false; }

    // Optional traits for Climate integration
    virtual void traits(void* /*traits*/, heat_pump_data_t& /*hp_data*/) {}

    // Public packet access
    uint8_t data_[256]{0};
    uint8_t data_len_{0};
    bool finalized_{false};
};

}  // namespace hwp
}  // namespace esphome



