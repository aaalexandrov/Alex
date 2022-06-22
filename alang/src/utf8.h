#pragma once

#include <cstdint>

namespace utf8 {

inline constexpr uint32_t InvalidCP = (uint32_t)-1;
inline constexpr uint32_t BomCP = 0xfeff;

uint8_t cp_size(uint32_t cp);
uint8_t cp_size(uint8_t *p);

uint32_t read_cp(uint8_t *&p, uint8_t cpSize = -1);
uint8_t write_cp(uint8_t *&p, uint32_t cp, uint8_t cpSize = -1);

}