#include "gem/utils/GEMRegisterUtils.h"

std::string gem::utils::uint32ToString(uint32_t const& val) {
  std::stringstream res;
  res <<(char)((val & (0xff000000)) / 16777216);
  res <<(char)((val & (0x00ff0000)) / 65536);
  res <<(char)((val & (0x0000ff00)) / 256);
  res <<(char)((val & (0x000000ff)));
  return res.str();
}

std::string gem::utils::uint32ToDottedQuad(uint32_t const& val) {
  std::stringstream res;
  res << (uint32_t)((val & (0xff000000)) / 16777216)<< std::dec << ".";
  res << (uint32_t)((val & (0x00ff0000)) / 65536)   << std::dec << ".";
  res << (uint32_t)((val & (0x0000ff00)) / 256)     << std::dec << ".";
  res << (uint32_t)((val & (0x000000ff)))           << std::dec;
  return res.str();
}

std::string gem::utils::uint32ToGroupedHex(uint32_t const& val1, uint32_t const& val2) {
  std::stringstream res;
  res << std::setfill('0') << std::setw(2) << std::hex
      <<(uint32_t)((val1 & (0x0000ff00)) / 256)     << std::dec << ":";
  res << std::setfill('0') << std::setw(2) << std::hex
      <<(uint32_t)((val1 & (0x000000ff)))           << std::dec << ":";
  res << std::setfill('0') << std::setw(2) << std::hex
      <<(uint32_t)((val2 & (0xff000000)) / 16777216)<< std::dec << ":";
  res << std::setfill('0') << std::setw(2) << std::hex
      <<(uint32_t)((val2 & (0x00ff0000)) / 65536)   << std::dec << ":";
  res << std::setfill('0') << std::setw(2) << std::hex
      <<(uint32_t)((val2 & (0x0000ff00)) / 256)     << std::dec << ":";
  res << std::setfill('0') << std::setw(2) << std::hex
      <<(uint32_t)((val2 & (0x000000ff)))           << std::dec;
  return res.str();
}
