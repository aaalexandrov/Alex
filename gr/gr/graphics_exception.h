#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace gr {

class GraphicsException : public std::runtime_error {
public:
  uint32_t _errCode;
  GraphicsException(char const *msg, uint32_t err) : std::runtime_error(msg), _errCode(err) {}
  GraphicsException(std::string const &msg, uint32_t err) : std::runtime_error(msg), _errCode(err) {}
};

}