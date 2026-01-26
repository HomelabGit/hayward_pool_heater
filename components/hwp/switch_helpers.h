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

#include "PoolHeater.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace hwp {

/**
 * @class PoolHeaterSwitch
 * @brief Base class for switches that control the state of a PoolHeater.
 */
class PoolHeaterSwitch : public switch_::Switch, public Component, public Parented<PoolHeater> {
  public:
    PoolHeaterSwitch() = default;

    // 2026 standard: Ensure parent is validated before use
    void setup() override {
        // Parented components in 2026 require checking for null to support 
        // the new Zero-Copy validation protocols.
        if (this->parent_ == nullptr) {
            this->mark_failed();
            return;
        }

        auto initial_state = this->get_initial_state_with_restore_mode().value_or(false);
        this->publish_state(initial_state);
        // In 2026, call the internal setter instead of write_state to avoid 
        // triggering duplicate hardware commands during boot.
        this->apply_initial_state_(initial_state);
    };

  protected:
    // Define the virtual method properly for 2026 standards
    void write_state(bool state) override = 0;
    
    // Helper for 2026 initial state application
    virtual void apply_initial_state_(bool state) = 0;
};

class ActiveModeSwitch : public PoolHeaterSwitch {
  protected:
    void write_state(bool state) override {
        this->parent_->set_passive_mode(!state);
        this->publish_state(state);
    }
    void apply_initial_state_(bool state) override {
        this->parent_->set_passive_mode(!state);
    }
};

class UpdateStatusSwitch : public PoolHeaterSwitch {
  protected:
    void write_state(bool state) override {
        this->parent_->set_update_active(state);
        this->publish_state(state);
    }
    void apply_initial_state_(bool state) override {
        this->parent_->set_update_active(state);
    }
};

} // namespace hwp
} // namespace esphome
