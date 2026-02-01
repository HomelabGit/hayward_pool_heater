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

#include "esphome/core/optional.h"
#include "esphome/core/log.h"
#include <vector>
#include <memory>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace esphome {
namespace hwp {

static const char* TAG_BF = "base_frame";

struct hp_packetdata_t {
    uint8_t data[12]{0};
    size_t data_len{0};

    bool operator!=(const hp_packetdata_t& other) const {
        if (data_len != other.data_len) return true;
        return std::memcmp(data, other.data, data_len) != 0;
    }
    uint8_t get_type() const { return data_len > 0 ? data[0] : 0; }

    bool is_checksum_valid() const;
    uint8_t bus_checksum() const;
    uint8_t calculate_checksum() const;
    std::string explain_checksum() const;
};

enum frame_source_t { SOURCE_UNKNOWN, SOURCE_HEATER, SOURCE_CONTROLLER, SOURCE_LOCAL };

class BaseFrame {
public:
    BaseFrame();
    BaseFrame(const BaseFrame& other);
    BaseFrame(BaseFrame&& other) noexcept;
    BaseFrame(const BaseFrame* other);

    uint8_t& operator[](size_t index);
    const uint8_t& operator[](size_t index) const;
    BaseFrame& operator=(const BaseFrame& other);

    static std::vector<BaseFrame>& get_registry();

    bool is_changed() const;
    bool is_size_valid() const;
    bool is_valid() const;
    bool is_checksum_valid() const;
    bool is_checksum_valid(bool& inverted) const;
    bool has_previous_data() const;
    bool is_short_frame() const;
    bool is_long_frame() const;

    size_t get_data_len() const;
    size_t get_type_id() const;
    frame_source_t get_source() const;
    void set_source(frame_source_t source);
    uint32_t get_frame_age_ms() const;
    uint32_t get_frame_time_ms() const;
    void set_frame_time_ms(uint32_t frame_time);
    void set_frame_time_ms();

    std::shared_ptr<BaseFrame> get_specialized();
    std::shared_ptr<BaseFrame> process(hp_packetdata_t& hp_data);

    void stage(const BaseFrame& base);
    void transfer();
    void parse(hp_packetdata_t& data);
    void initialize();

    void debug_print_hex() const;
    template <size_t N>
    void debug_print_hex(const uint8_t (&buffer)[N], const size_t length, frame_source_t source);

    void print(const std::string& prefix, const char* tag, int min_level, int line) const;
    void print_prev(const std::string& prefix, const char* tag, int min_level, int line) const;

    std::string format(bool no_diff = false) const;
    std::string format_prev() const;
    std::string to_string(const std::string& prefix = "") const;

    static uint8_t reverse_bits(unsigned char x);
    static const char* source_string(frame_source_t source);

private:
    hp_packetdata_t packet;
    optional<hp_packetdata_t> prev_;
    size_t transmitBitIndex{0};
    bool finalized{false};
    frame_source_t source_{SOURCE_UNKNOWN};
    uint32_t frame_time_ms_{0};
    size_t type_id_{0};

    static std::vector<BaseFrame> registry_;
};

}  // namespace hwp
}  // namespace esphome

