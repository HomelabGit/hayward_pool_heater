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

#include "CS.h"
#include "Schema.h"
#include "base_frame.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace esphome {
namespace hwp {

/**
 * @brief Represents the time-related payload of the clock frame.
 *
 * Note: year/month/day appear to be counters/offsets, not true calendar values.
 */
typedef struct clock_time {
  uint8_t id;
  uint8_t reserved1;
  uint8_t reserved2;
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t reserved3;
  uint8_t reserved4;

  std::time_t decode() const {
    std::tm tm_time{};
    tm_time.tm_year = static_cast<int>(this->year);   // beware: not real year
    tm_time.tm_mon  = static_cast<int>(this->month);  // 0..11 normally
    tm_time.tm_mday = static_cast<int>(this->day);
    tm_time.tm_hour = static_cast<int>(this->hour);
    tm_time.tm_min  = static_cast<int>(this->minute);
    tm_time.tm_sec  = 0;
    return std::mktime(&tm_time);
  }

  std::string format() const {
    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << static_cast<int>(this->year) << "/";
    oss << std::setw(2) << std::setfill('0') << static_cast<int>(this->month) << "/";
    oss << std::setw(2) << std::setfill('0') << static_cast<int>(this->day) << " - ";
    oss << std::setw(2) << std::setfill('0') << static_cast<int>(this->hour) << ":";
    oss << std::setw(2) << std::setfill('0') << static_cast<int>(this->minute);
    return oss.str();
  }

  std::string diff(const clock_time &reference, const std::string &separator = "") const {
    const std::string ref = reference.format();
    const std::string cur = this->format();

    CS cs;
    const bool changed = (ref != cur);
    cs.set_changed_base_color(changed);

    const char *cs_inv = changed ? CS::invert : "";
    const char *cs_inv_rst = changed ? CS::invert_rst : "";

    const size_t n = (ref.size() < cur.size()) ? ref.size() : cur.size();
    for (size_t i = 0; i < n; i++) {
      if (ref[i] != cur[i]) {
        cs << cs_inv << cur[i] << cs_inv_rst;  // show CURRENT char when changed
      } else {
        cs << cur[i];
      }
    }
    // If lengths differ (shouldn't), append the remainder safely.
    if (cur.size() > n) cs << cur.substr(n);

    cs << separator;
    return cs.str();
  }

  bool operator==(const clock_time &other) const {
    return this->id == other.id &&
           this->year == other.year &&
           this->month == other.month &&
           this->day == other.day &&
           this->hour == other.hour &&
           this->minute == other.minute;
  }
  bool operator!=(const clock_time &other) const { return !(*this == other); }

} __attribute__((packed)) clock_time_t;

static_assert(sizeof(clock_time_t) == frame_data_length - 2, "frame structure has wrong size");

class FrameClock : public BaseFrame {
 public:
  CLASS_DEFAULT_IMPL(FrameClock, clock_time_t);

  static constexpr uint8_t FRAME_ID_CLOCK = 0xCF;  // Clock
};

}  // namespace hwp
}  // namespace esphome

