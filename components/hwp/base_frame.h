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
#include "HPUtils.h"
#include "Schema.h"
#include "hwp_call.h"

#include "esphome/components/climate/climate.h"
#include "esphome/components/logger/logger.h"
#include "esphome/core/helpers.h"   // esphome::millis()
#include "esphome/core/log.h"

#include <bitset>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>

// NOTE: ESP-IDF prints a deprecation warning if you include legacy driver/rmt.h.
// Keep it if you still use the legacy API. Otherwise switch to driver/rmt_rx.h / rmt_tx.h.
#include <driver/rmt.h>

namespace esphome {
namespace hwp {

// -----------------------------------------------------------------------------
// Frame registration macros
// -----------------------------------------------------------------------------
//
// Key change vs your current macro:
//   - remove the undefined DATA_T symbol
//   - stop trying to assign packet_data -> optional<T> directly
//   - always decode using packet.as_type<type_name>() (already exists in Schema.h)
//
// This fixes:
//   - "DATA_T was not declared in this scope"
//   - ambiguous operator= from optional<T> = packet_data
//
#define CLASS_DEFAULT_IMPL(DerivedFrameClass, type_name)                                             \
  static size_t class_type_id;                                                                       \
  DerivedFrameClass() : BaseFrame(), data_(), prev_data_() {}                                         \
  DerivedFrameClass(const BaseFrame &base) : BaseFrame(base), data_(), prev_data_() {                 \
    /* stage() will populate data_ properly */                                                        \
    this->stage(base);                                                                               \
  }                                                                                                   \
  DerivedFrameClass(const DerivedFrameClass &other)                                                   \
      : BaseFrame(other), data_(other.data_), prev_data_(other.prev_data_) {}                         \
  template <size_t N>                                                                                \
  DerivedFrameClass(const unsigned char (&cmdTrame)[N]) : BaseFrame(cmdTrame), data_(), prev_data_() {\
    /* decode from packet bytes into strongly-typed struct */                                         \
    data_ = esphome::optional<type_name>(this->packet.template as_type<type_name>());                \
  }                                                                                                   \
  static std::shared_ptr<BaseFrame> create();                                                         \
  static bool matches(BaseFrame &specialized, BaseFrame &base);                                       \
  bool matches(BaseFrame &base) { return matches(*this, base); }                                      \
  esphome::optional<type_name> data_;                                                                 \
  esphome::optional<type_name> prev_data_;                                                            \
  type_name &data() { return *data_; }                                                                \
  size_t get_type_id() const override { return type_id_; }                                            \
  void stage(const BaseFrame &base) override {                                                        \
    BaseFrame::stage(base);                                                                           \
    /* decode after copying packet */                                                                 \
    this->data_ = esphome::optional<type_name>(this->packet.template as_type<type_name>());          \
  }                                                                                                   \
  esphome::optional<std::shared_ptr<BaseFrame>> control(const HWPCall &call) override;                \
  void transfer() override {                                                                          \
    if (this->data_.has_value()) this->prev_data_ = this->data_;                                      \
  }                                                                                                   \
  std::string format(const type_name &val, const type_name &ref) const;                               \
  std::string format(bool no_diff = false) const override;                                            \
  std::string format_prev() const override;                                                           \
  const char *type_string() const override;                                                           \
  bool is_changed() const override {                                                                  \
    return !prev_data_.has_value() || !data_.has_value() || (*this->data_ != this->prev_data_.value());\
  }                                                                                                   \
  bool has_previous_data() const override {                                                           \
    return data_.has_value() && prev_data_.has_value();                                               \
  }                                                                                                   \
  void finalize() {                                                                                   \
    if (this->data_.has_value()) this->packet.from_type(*this->data_);                                \
  }                                                                                                   \
  void parse(heat_pump_data_t &hp_data) override;

// Define the macro to accept a fully qualified class name.
#define CLASS_ID_DECLARATION(FullClassName)                                                           \
  size_t FullClassName::class_type_id =                                                               \
      BaseFrame::register_frame_class(&FullClassName::create, &FullClassName::matches);

#define REGISTER_FRAME_ID_DEFAULT(DerivedFrameClass)

// -----------------------------------------------------------------------------
// Enums / constants
// -----------------------------------------------------------------------------
typedef enum { SOURCE_UNKNOWN, SOURCE_HEATER, SOURCE_CONTROLLER, SOURCE_LOCAL } frame_source_t;

static constexpr uint16_t pulse_duration_threshold_us = 600;
static constexpr uint32_t frame_heading_low_duration_ms = 9;
static constexpr uint32_t frame_heading_high_duration_ms = 5;
static constexpr uint32_t bit_long_high_duration_ms = 3;
static constexpr uint32_t bit_low_duration_ms = 1;
static constexpr uint32_t bit_short_high_duration_ms = bit_low_duration_ms;
static constexpr uint32_t frame_end_threshold_ms = 50;           // spacing between each frame is 100ms.
static constexpr uint32_t controler_group_spacing_ms = 250;
static constexpr uint32_t controler_frame_spacing_duration_ms = 100;
static constexpr uint32_t delay_between_sending_messages_ms = 10 * 1000; // restrict changes to once per 10 seconds
static constexpr uint32_t delay_between_controller_messages_ms = 60 * 1000;

static constexpr const char TAG_BF[] = "hwp";
static constexpr const char TAG_BF_HEX[] = "hwp.hex";

static constexpr uint32_t frame_heading_total_duration_ms =
    frame_heading_low_duration_ms + frame_heading_high_duration_ms;

// -----------------------------------------------------------------------------
// BaseFrame
// -----------------------------------------------------------------------------
class BaseFrame {
 public:
  using FrameFactoryMethod = std::shared_ptr<BaseFrame> (*)();
  using FrameMatchesMethod = bool (*)(BaseFrame &specialized, BaseFrame &base);

  typedef struct {
    FrameFactoryMethod factory;
    FrameMatchesMethod matches;
    std::shared_ptr<BaseFrame> instance;
  } frame_registry_t;

  static std::vector<frame_registry_t> &get_registry();
  static std::shared_ptr<BaseFrame> base_create();
  static bool base_matches(BaseFrame &specialized, BaseFrame &base);
  static size_t register_frame_class(FrameFactoryMethod factory, FrameMatchesMethod match_method);

  BaseFrame();
  BaseFrame(const BaseFrame &other);
  BaseFrame(BaseFrame &&other) noexcept;
  BaseFrame(const BaseFrame *other);

  template <size_t N>
  BaseFrame(const unsigned char (&base_data)[N]) : BaseFrame() {
    *this = base_data;
  }

  virtual ~BaseFrame() = default;

  BaseFrame &operator=(const BaseFrame &other);

  template <typename T>
  std::shared_ptr<T> operator()();

  template <size_t N>
  BaseFrame &operator=(const unsigned char (&base_data)[N]) {
    // Copy raw bytes into packet. Assumes packet_data has fixed storage.
    // Keep existing behaviour: first byte is type; last is checksum; etc.
    packet.data_len = (N <= sizeof(packet.data)) ? N : sizeof(packet.data);
    std::memcpy(packet.data, base_data, packet.data_len);
    this->set_frame_time_ms();
    return *this;
  }

  uint8_t &operator[](size_t index);
  const uint8_t &operator[](size_t index) const;

  virtual void initialize();
  virtual void parse(heat_pump_data_t &data);

  virtual bool has_previous_data() const;
  virtual std::string format(bool no_diff = false) const;
  virtual std::string format_prev() const;
  virtual size_t get_type_id() const;
  virtual bool is_changed() const;
  virtual const char *type_string() const;

  virtual esphome::optional<std::shared_ptr<BaseFrame>> control(const HWPCall &call);
  virtual void traits(climate::ClimateTraits &traits, heat_pump_data_t &hp_data) {}

  bool is_long_frame() const;
  size_t get_data_len() const;
  bool is_short_frame() const;
  bool is_type_id(const BaseFrame &frame) const;

  void set_frame_time_ms();
  void set_frame_time_ms(uint32_t frame_time);

  bool is_checksum_valid() const;
  bool is_checksum_valid(bool &inverted) const;

  uint32_t get_frame_age_ms() const;
  uint32_t get_frame_time_ms() const;

  uint8_t *data();

  std::string format(const hp_packetdata_t &val, const hp_packetdata_t &ref) const;

  static const char *format_hex(uint8_t val);
  static const char *format_hex_diff(uint8_t val, uint8_t ref);

  static void print(const std::string &prefix, const BaseFrame &frame, const char *tag,
                    int min_level, int line);
  void print(const std::string &prefix, const char *tag, int min_level, int line) const;

  static void print_diff(const std::string &prefix, const BaseFrame &frame, const char *tag,
                         int min_level, int line);
  static void print_prev(const std::string &prefix, const BaseFrame &frame, const char *tag,
                         int min_level, int line);

  void print_prev(const std::string &prefix, const char *tag, int min_level, int line) const;
  void print_diff(const std::string &prefix, const char *tag, int min_level, int line) const;

  std::string header_format(const std::string &prefix, bool no_diff = false) const;

  static const char *source_string(frame_source_t source);
  frame_source_t get_source() const;
  void set_source(frame_source_t source);
  const char *source_string() const;

  // IMPORTANT: ESPHome 2025/2026 logger.h warns about Logger::level_for inline.
  // Keeping this here triggers your warning. So make it safe: do NOT call level_for().
  static inline bool log_active(const char * /*tag*/,
                                int /*min_level*/ = ESPHOME_LOG_LEVEL_VERBOSE) {
    return logger::global_logger != nullptr;
  }

  static void dump_known_packets(const char *CALLER_TAG);

  template <size_t N>
  static void debug_print_hex(const uint8_t (&buffer)[N], const size_t length,
                              const frame_source_t source);
  void debug_print_hex() const;

  size_t size() const;
  bool is_size_valid() const;
  bool is_valid() const;

  void inverse();
  std::string to_string(const std::string &prefix) const;

  template <typename T>
  static std::shared_ptr<T> get() {
    auto &registry = get_registry();
    if (T::class_type_id < registry.size()) {
      return std::static_pointer_cast<T>(registry[T::class_type_id].instance);
    }
    return nullptr;
  }

  hp_packetdata_t packet;
  size_t transmitBitIndex;
  bool finalized;

  static uint8_t reverse_bits(unsigned char x);
  static void dump_c_code(const char *caller_tag);

 protected:
  frame_source_t source_;
  uint32_t frame_time_ms_;
  uint32_t frame_age_ms_;
  size_t type_id_ = 0;

  esphome::optional<uint8_t> byte_signature_ = 0;
  esphome::optional<hp_packetdata_t> prev_;

  static std::vector<frame_registry_t> registry_;

  virtual void transfer();
  virtual void stage(const BaseFrame &base);

  std::shared_ptr<BaseFrame> get_specialized();
  std::shared_ptr<BaseFrame> process(heat_pump_data_t &hp_data);

  frame_registry_t *get_registry_by_id(size_t type_id);
};

}  // namespace hwp
}  // namespace esphome
