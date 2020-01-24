/**
 *
 */

#include "gem/hw/utils/GEMCrateUtils.h"

#include <bitset>

#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/format.hpp"

// #include "gem/utils/GEMLogging.h"

uint16_t gem::hw::utils::parseAMCEnableList(std::string const& enableList)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMCrateUtilsLogger"));
  return parseAMCEnableList(enableList, m_gemLogger);
}

// uint16_t gem::hw::utils::parseAMCEnableList(std::string const& enableList, log4cplus::Logger* logger=nullptr)
// {
//   if (logger == nullptr) {
//     std::cout << "parseAMCEnableList creating m_gemLogger anew" << std::endl;
//     log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMCrateUtilsLogger"));
//   } else {
//     std::cout << "parseAMCEnableList creating m_gemLogger from " << std::hex << logger << std::dec
//               << ", with name " << logger->getName() << std::endl;
//     log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance(logger->getName()+".child"));
//   }

uint16_t gem::hw::utils::parseAMCEnableList(std::string const& enableList, log4cplus::Logger logger)
{
  log4cplus::Logger m_gemLogger(logger);
  uint16_t slotMask = 0x0;
  std::vector<std::string> slots;

  boost::split(slots, enableList, boost::is_any_of(", "), boost::token_compress_on);
  CMSGEMOS_DEBUG("GEMCrateUtils::parseAMCEnableList::AMC input enable list is " << enableList);
  // would be great to multithread this portion
  for (auto slot = slots.begin(); slot != slots.end(); ++slot) {
    CMSGEMOS_DEBUG("GEMCrateUtils::parseAMCEnableList::slot is " << *slot);
    if (slot->find('-') != std::string::npos) {  // found a possible range
      CMSGEMOS_DEBUG("GEMCrateUtils::parseAMCEnableList::found a hyphen in " << *slot);
      std::vector<std::string> range;
      boost::split(range, *slot, boost::is_any_of("-"), boost::token_compress_on);
      if (range.size() > 2) {
        CMSGEMOS_WARN("GEMCrateUtils::parseAMCEnableList::Found poorly formatted range " << *slot);
        continue;
      }
      if (isValidSlotNumber(HWType::uTCA, range.at(0)) && isValidSlotNumber(HWType::uTCA, range.at(1))) {
        std::stringstream ss0(range.at(0));
        std::stringstream ss1(range.at(1));
        int min, max;
        ss0 >> min;
        ss1 >> max;

        if (min == max) {
          CMSGEMOS_WARN("GEMCrateUtils::parseAMCEnableList::Found poorly formatted range " << *slot);
          continue;
        }
        if (min > max) {  // elements in the wrong order
          CMSGEMOS_WARN("GEMCrateUtils::parseAMCEnableList::Found poorly formatted range " << *slot);
          continue;
        }

        for (int islot = min; islot <= max; ++islot) {
          slotMask |= (0x1 << (islot-1));
        }  //  end loop over range of list
      }  // end check on valid values
    } else {  //not a range
      CMSGEMOS_DEBUG("GEMCrateUtils::parseAMCEnableList::found no hyphen in " << *slot);
      if (slot->length() > 2) {
        CMSGEMOS_WARN("GEMCrateUtils::parseAMCEnableList::Found longer value than expected (1-12) " << *slot);
        continue;
      }

      if (!isValidSlotNumber(HWType::uTCA, *slot)) {
        CMSGEMOS_WARN("GEMCrateUtils::parseAMCEnableList::Found invalid value " << *slot);
        continue;
      }
      std::stringstream ss(*slot);
      int slotNum = -1;
      ss >> slotNum;
      slotMask |= (0x1 << (slotNum-1));
    }  // done processing single values
  }  // done looping over extracted values
  CMSGEMOS_DEBUG("GEMCrateUtils::parseAMCEnableList::Parsed enabled list 0x" << std::hex << slotMask << std::dec);
  return slotMask;
}


uint32_t gem::hw::utils::parseVFATMaskList(std::string const& enableList)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMCrateUtilsLogger"));
  return parseVFATMaskList(enableList, m_gemLogger);
}

uint32_t gem::hw::utils::parseVFATMaskList(std::string const& enableList, log4cplus::Logger logger)
{
  log4cplus::Logger m_gemLogger(logger);

  // nothing masked, return the negation of the mask that includes the enable list
  uint32_t broadcastMask = 0x00000000;
  std::vector<std::string> slots;

  boost::split(slots, enableList, boost::is_any_of(", "), boost::token_compress_on);
  CMSGEMOS_DEBUG("GEMCrateUtils::parseVFATMaskList::VFAT broadcast enable list is " << enableList);
  for (auto slot = slots.begin(); slot != slots.end(); ++slot) {
    CMSGEMOS_DEBUG("GEMCrateUtils::parseVFATMaskList::slot is " << *slot);
    if (slot->find('-') != std::string::npos) {  // found a possible range
      CMSGEMOS_DEBUG("GEMCrateUtils::parseVFATMaskList::found a hyphen in " << *slot);
      std::vector<std::string> range;
      boost::split(range, *slot, boost::is_any_of("-"), boost::token_compress_on);
      if (range.size() > 2) {
        CMSGEMOS_WARN("GEMCrateUtils::parseVFATMaskList::Found poorly formatted range " << *slot);
        continue;
      }
      if (isValidSlotNumber(HWType::GEB, range.at(0)) && isValidSlotNumber(HWType::GEB, range.at(1))) {
        std::stringstream ss0(range.at(0));
        std::stringstream ss1(range.at(1));
        int min, max;
        ss0 >> min;
        ss1 >> max;

        if (min == max) {
          CMSGEMOS_WARN("GEMCrateUtils::parseVFATMaskList::Found poorly formatted range " << *slot);
          continue;
        }
        if (min > max) {  // elements in the wrong order
          CMSGEMOS_WARN("GEMCrateUtils::parseVFATMaskList::Found poorly formatted range " << *slot);
          continue;
        }

        for (int islot = min; islot <= max; ++islot) {
          broadcastMask |= (0x1 << (islot));
        }  //  end loop over range of list
      }  // end check on valid values
    } else {  //not a range
      CMSGEMOS_DEBUG("GEMCrateUtils::parseVFATMaskList::found no hyphen in " << *slot);
      if (slot->length() > 2) {
        CMSGEMOS_WARN("GEMCrateUtils::parseVFATMaskList::Found longer value than expected (0-23) " << *slot);
        continue;
      }

      if (!isValidSlotNumber(HWType::GEB, *slot)) {
        CMSGEMOS_WARN("GEMCrateUtils::parseVFATMaskList::Found invalid value " << *slot);
        continue;
      }
      std::stringstream ss(*slot);
      int slotNum = -1;
      ss >> slotNum;
      broadcastMask |= (0x1 << (slotNum));
    }  //done processing single values
  }  //done looping over extracted values

  CMSGEMOS_DEBUG("GEMCrateUtils::parseVFATMaskList::Parsed enabled list 0x" << std::hex << broadcastMask << std::dec
        << " bits set " << std::bitset<32>(broadcastMask).count()
        << " inverted: 0x" << std::hex << ~broadcastMask << std::dec
        << " bits set " << std::bitset<32>(~broadcastMask).count()
        );
  return ~broadcastMask;
}


bool gem::hw::utils::isValidSlotNumber(HWType const& type, std::string const& s)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMCrateUtilsLogger"));
  return isValidSlotNumber(type, s, m_gemLogger);
}

bool gem::hw::utils::isValidSlotNumber(HWType const& type, std::string const& s, log4cplus::Logger logger)
{
  log4cplus::Logger m_gemLogger(logger);

  try {
    int i_val;
    int rangeMin, rangeMax;

    if (type == HWType::uTCA) {
      rangeMin = 1;
      rangeMax = 12;
    } else if (type == HWType::GEB) {
      rangeMin = 0;
      rangeMax = 23;
    } else {
      CMSGEMOS_ERROR("GEMCrateUtils::isValidSlotNumber::invalid HWType specified " << (int)type);
      return false;
    }

    i_val = std::stoi(s);
    if (!(i_val >= rangeMin && i_val <= rangeMax)) {
      CMSGEMOS_ERROR("GEMCrateUtils::isValidSlotNumber::Found value outside expected ("
            << rangeMin << " - " << rangeMax << ") " << i_val);
      return false;
    }
  } catch (std::invalid_argument const& err) {
    CMSGEMOS_ERROR("GEMCrateUtils::isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  } catch (std::out_of_range const& err) {
    CMSGEMOS_ERROR("GEMCrateUtils::isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  }
  // if you get here, should be possible to parse as an integer in the range [rangeMin, rangeMax]
  return true;
}


double gem::hw::utils::scaConversion(gem::hw::utils::SCAType const& type,
                                     uint32_t const& val)
{
  switch(type) {
  case(gem::hw::utils::SCAType::ADC_V):
    return static_cast<double>(val*(0.244/1000)*3.0);
  case(gem::hw::utils::SCAType::ADC_T):
    return static_cast<double>(val*(-0.53)+381.2);
  case(gem::hw::utils::SCAType::PT100):
    return static_cast<double>(0.2597*(9891.8-2.44*val));
  case(gem::hw::utils::SCAType::SYSMON_V):
    return static_cast<double>(val*0.49-273.15);
  case(gem::hw::utils::SCAType::SYSMON_T):
    return static_cast<double>(val*2.93/1000);
  default:
    return -1.0;
  }
}
