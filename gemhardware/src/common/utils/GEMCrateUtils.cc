/**
 *
 */

#include <bitset>

#include "gem/hw/utils/GEMCrateUtils.h"

#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/format.hpp"

#include "gem/utils/GEMLogging.h"

uint16_t gem::hw::utils::parseAMCEnableList(std::string const& enableList)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMCrateUtilsLogger"));

  uint16_t slotMask = 0x0;
  std::vector<std::string> slots;

  boost::split(slots, enableList, boost::is_any_of(", "), boost::token_compress_on);
  DEBUG("GEMCrateUtils::parseAMCEnableList::AMC input enable list is " << enableList);
  // would be great to multithread this portion
  for (auto slot = slots.begin(); slot != slots.end(); ++slot) {
    DEBUG("GEMCrateUtils::parseAMCEnableList::slot is " << *slot);
    if (slot->find('-') != std::string::npos) {  // found a possible range
      DEBUG("GEMCrateUtils::parseAMCEnableList::found a hyphen in " << *slot);
      std::vector<std::string> range;
      boost::split(range, *slot, boost::is_any_of("-"), boost::token_compress_on);
      if (range.size() > 2) {
        WARN("GEMCrateUtils::parseAMCEnableList::Found poorly formatted range " << *slot);
        continue;
      }
      if (isValidSlotNumber(HWType::uTCA, range.at(0)) && isValidSlotNumber(HWType::uTCA, range.at(1))) {
        std::stringstream ss0(range.at(0));
        std::stringstream ss1(range.at(1));
        int min, max;
        ss0 >> min;
        ss1 >> max;

        if (min == max) {
          WARN("GEMCrateUtils::parseAMCEnableList::Found poorly formatted range " << *slot);
          continue;
        }
        if (min > max) {  // elements in the wrong order
          WARN("GEMCrateUtils::parseAMCEnableList::Found poorly formatted range " << *slot);
          continue;
        }

        for (int islot = min; islot <= max; ++islot) {
          slotMask |= (0x1 << (islot-1));
        }  //  end loop over range of list
      }  // end check on valid values
    } else {  //not a range
      DEBUG("GEMCrateUtils::parseAMCEnableList::found no hyphen in " << *slot);
      if (slot->length() > 2) {
        WARN("GEMCrateUtils::parseAMCEnableList::Found longer value than expected (1-12) " << *slot);
        continue;
      }

      if (!isValidSlotNumber(HWType::uTCA, *slot)) {
        WARN("GEMCrateUtils::parseAMCEnableList::Found invalid value " << *slot);
        continue;
      }
      std::stringstream ss(*slot);
      int slotNum = -1;
      ss >> slotNum;
      slotMask |= (0x1 << (slotNum-1));
    }  // done processing single values
  }  // done looping over extracted values
  DEBUG("GEMCrateUtils::parseAMCEnableList::Parsed enabled list 0x" << std::hex << slotMask << std::dec);
  return slotMask;
}


uint32_t gem::hw::utils::parseVFATMaskList(std::string const& enableList)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMCrateUtilsLogger"));

  // nothing masked, return the negation of the mask that includes the enable list
  uint32_t broadcastMask = 0x00000000;
  std::vector<std::string> slots;

  boost::split(slots, enableList, boost::is_any_of(", "), boost::token_compress_on);
  DEBUG("GEMCrateUtils::parseVFATMaskList::VFAT broadcast enable list is " << enableList);
  for (auto slot = slots.begin(); slot != slots.end(); ++slot) {
    DEBUG("GEMCrateUtils::parseVFATMaskList::slot is " << *slot);
    if (slot->find('-') != std::string::npos) {  // found a possible range
      DEBUG("GEMCrateUtils::parseVFATMaskList::found a hyphen in " << *slot);
      std::vector<std::string> range;
      boost::split(range, *slot, boost::is_any_of("-"), boost::token_compress_on);
      if (range.size() > 2) {
        WARN("GEMCrateUtils::parseVFATMaskList::Found poorly formatted range " << *slot);
        continue;
      }
      if (isValidSlotNumber(HWType::GEB, range.at(0)) && isValidSlotNumber(HWType::GEB, range.at(1))) {
        std::stringstream ss0(range.at(0));
        std::stringstream ss1(range.at(1));
        int min, max;
        ss0 >> min;
        ss1 >> max;

        if (min == max) {
          WARN("GEMCrateUtils::parseVFATMaskList::Found poorly formatted range " << *slot);
          continue;
        }
        if (min > max) {  // elements in the wrong order
          WARN("GEMCrateUtils::parseVFATMaskList::Found poorly formatted range " << *slot);
          continue;
        }

        for (int islot = min; islot <= max; ++islot) {
          broadcastMask |= (0x1 << (islot));
        }  //  end loop over range of list
      }  // end check on valid values
    } else {  //not a range
      DEBUG("GEMCrateUtils::parseVFATMaskList::found no hyphen in " << *slot);
      if (slot->length() > 2) {
        WARN("GEMCrateUtils::parseVFATMaskList::Found longer value than expected (0-23) " << *slot);
        continue;
      }

      if (!isValidSlotNumber(HWType::GEB, *slot)) {
        WARN("GEMCrateUtils::parseVFATMaskList::Found invalid value " << *slot);
        continue;
      }
      std::stringstream ss(*slot);
      int slotNum = -1;
      ss >> slotNum;
      broadcastMask |= (0x1 << (slotNum));
    }  //done processing single values
  }  //done looping over extracted values

  DEBUG("GEMCrateUtils::parseVFATMaskList::Parsed enabled list 0x" << std::hex << broadcastMask << std::dec
        << " bits set " << std::bitset<32>(broadcastMask).count()
        << " inverted: 0x" << std::hex << ~broadcastMask << std::dec
        << " bits set " << std::bitset<32>(~broadcastMask).count()
        );
  return ~broadcastMask;
}

bool gem::hw::utils::isValidSlotNumber(HWType const& type, std::string const& s)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMCrateUtilsLogger"));

  try {
    int i_val;

    int rangeMin,rangeMax;

    if (type == HWType::uTCA) {
      rangeMin = 1;
      rangeMax = 12;
    } else if (type == HWType::GEB) {
      rangeMin = 0;
      rangeMax = 23;
    } else {
      ERROR("GEMCrateUtils::isValidSlotNumber::invalid HWType specified " << (int)type);
      return false;
    }

    i_val = std::stoi(s);
    if (!(i_val >= rangeMin && i_val <= rangeMax)) {
      ERROR("GEMCrateUtils::isValidSlotNumber::Found value outside expected ("
            << rangeMin << " - " << rangeMax << ") " << i_val);
      return false;
    }
  } catch (std::invalid_argument const& err) {
    ERROR("GEMCrateUtils::isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  } catch (std::out_of_range const& err) {
    ERROR("GEMCrateUtils::isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  }
  // if you get here, should be possible to parse as an integer in the range [rangeMin, rangeMax]
  return true;
}
