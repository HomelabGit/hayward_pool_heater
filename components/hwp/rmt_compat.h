#pragma once

// Prefer modern ESP-IDF RMT headers when available; fall back to legacy.
#if __has_include(<driver/rmt_tx.h>) && __has_include(<driver/rmt_rx.h>) && \
    __has_include(<hal/rmt_types.h>)
#include <driver/rmt_rx.h>
#include <driver/rmt_tx.h>
#include <hal/rmt_types.h>
namespace esphome {
namespace hwp {
using hwp_rmt_item_t = rmt_symbol_word_t;
}  // namespace hwp
}  // namespace esphome
#elif __has_include(<driver/rmt.h>)
#include <driver/rmt.h>
namespace esphome {
namespace hwp {
using hwp_rmt_item_t = rmt_item32_t;
}  // namespace hwp
}  // namespace esphome
#else
#error "No compatible ESP-IDF RMT header found."
#endif
