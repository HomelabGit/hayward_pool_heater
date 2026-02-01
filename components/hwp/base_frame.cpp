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
#include "base_frame.h"

namespace esphome {
namespace hwp {

// Static registry
std::vector<BaseFrame> BaseFrame::registry_;

// Constructors
BaseFrame::BaseFrame() : frame_time_ms_(millis()) {}
BaseFrame::BaseFrame(const BaseFrame& other)
    : packet(other.packet), transmitBitIndex(0), finalized(other.finalized),
      source_(other.source_), frame_time_ms_(other.frame_time_ms_) {}
BaseFrame::BaseFrame(BaseFrame&& other) noexcept
    : packet(std::move(other.packet)), transmitBitIndex(0), finalized(other.finalized),
      source_(other.source_), frame_time_ms_(other.frame_time_ms_) {}
BaseFrame::BaseFrame(const BaseFrame* other) {
    if (other != nullptr) {
        packet = other->packet;
        transmitBitIndex = 0;
        source_ = other->source_;
        frame_time_ms_ = other->frame_time_ms_;
        finalized = other->finalized;
    }
}

// Access operators
uint8_t& BaseFrame::operator[](size_t index) { return packet.data[index]; }
const uint8_t& BaseFrame::operator[](size_t index) const { return packet.data[index]; }
BaseFrame& BaseFrame::operator=(const BaseFrame& other) {
    if (this != &other) {
        packet = other.packet;
        source_ = other.source_;
        frame_time_ms_ = other.frame_time_ms_;
        finalized = other.finalized;
    }
    return *this;
}

// Status checks
bool BaseFrame::is_changed() const { return !has_previous_data() || packet != prev_.value_or(packet); }
bool BaseFrame::is_valid() const { return source_ != SOURCE_UNKNOWN; }
bool BaseFrame::is_size_valid() const { return packet.data_len == 12 || packet.data_len == 6; }
bool BaseFrame::is_short_frame() const { return packet.data_len == 6; }
bool BaseFrame::is_long_frame() const { return packet.data_len == 12; }

// Frame time
uint32_t BaseFrame::get_frame_age_ms() const { return millis() - frame_time_ms_; }
uint32_t BaseFrame::get_frame_time_ms() const { return frame_time_ms_; }
void BaseFrame::set_frame_time_ms(uint32_t frame_time) { frame_time_ms_ = frame_time; }
void BaseFrame::set_frame_time_ms() { frame_time_ms_ = millis(); }

// Source
frame_source_t BaseFrame::get_source() const { return source_; }
void BaseFrame::set_source(frame_source_t source) { source_ = source; }
const char* BaseFrame::source_string(frame_source_t source) {
    switch (source) {
        case SOURCE_CONTROLLER: return "CONT";
        case SOURCE_HEATER: return "HEAT";
        case SOURCE_LOCAL: return "LOC ";
        default: return "UNK ";
    }
}
const char* BaseFrame::source_string() const { return source_string(source_); }

// Utilities
uint8_t BaseFrame::reverse_bits(unsigned char x) {
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
    return x;
}

// Debug print
void BaseFrame::debug_print_hex() const {
    char buffer[128];
    size_t len = snprintf(buffer, sizeof(buffer), "%s: ", source_string());
    for (size_t i = 0; i < packet.data_len; i++) {
        len += snprintf(buffer + len, sizeof(buffer) - len, "%02X ", packet.data[i]);
    }
    ESP_LOGD(TAG_BF, "%s", buffer);
}

template <size_t N>
void BaseFrame::debug_print_hex(const uint8_t (&buffer)[N], const size_t length, frame_source_t source) {
    char line[128];
    size_t len = snprintf(line, sizeof(line), "%s: ", source_string(source));
    for (size_t i = 0; i < length && i < N; i++) {
        len += snprintf(line + len, sizeof(line) - len, "%02X ", buffer[i]);
    }
    ESP_LOGD(TAG_BF, "%s", line);
}

// Format functions
std::string BaseFrame::format(bool no_diff) const {
    char buf[128];
    snprintf(buf, sizeof(buf), "TYPE_%02X (%s) len=%d", packet.get_type(), source_string(), packet.data_len);
    return std::string(buf);
}

std::string BaseFrame::format_prev() const {
    if (!prev_.has_value()) return "N/A";
    return format(false);
}

// Print functions
void BaseFrame::print(const std::string& prefix, const char* tag, int min_level, int line) const {
    if (!log_active(tag, min_level)) return;
    ESP_LOGD(tag, "%s %s", prefix.c_str(), format().c_str());
}
void BaseFrame::print_prev(const std::string& prefix, const char* tag, int min_level, int line) const {
    if (!log_active(tag, min_level)) return;
    ESP_LOGD(tag, "%s %s", prefix.c_str(), format_prev().c_str());
}

// Stage / transfer
void BaseFrame::stage(const BaseFrame& base) { packet = base.packet; source_ = base.source_; finalized = base.finalized; }
void BaseFrame::transfer() {}

// Specialized processing
std::shared_ptr<BaseFrame> BaseFrame::get_specialized() {
    if (registry_.empty()) return std::make_shared<BaseFrame>();
    return std::make_shared<BaseFrame>(registry_.front());
}

std::shared_ptr<BaseFrame> BaseFrame::process(hp_packetdata_t& hp_data) {
    auto specialized = get_specialized();
    if (!specialized) return nullptr;
    specialized->stage(*this);
    specialized->print("Proc", TAG_BF, ESPHOME_LOG_LEVEL_DEBUG, __LINE__);
    return specialized;
}

}  // namespace hwp
}  // namespace esphome
