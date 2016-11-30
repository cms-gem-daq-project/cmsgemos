/** @file GEMStringsToEnums.h */

#ifndef GEM_BASE_GEMSTRINGSTOENUMS_H
#define GEM_BASE_GEMSTRINGSTOENUMS_H

#include "gem/base/GEMEnums.h"

#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>

namespace gem {
  namespace base {
    typedef boost::unordered_map<std::string, GEMRunType::EGEMRunType >   stringToGEMRunType;
    typedef boost::unordered_map<std::string, GEMScanType::EGEMScanType > stringToGEMScanType;

    const stringToGEMRunType StringToGEMRunType = boost::assign::map_list_of
      ("DATA"     , GEMRunType::DATA     )
      ("COSMIC"   , GEMRunType::COSMIC   )
      ("TESTSTAND", GEMRunType::TESTSTAND)
      ("SCAN"     , GEMRunType::SCAN     );

    const stringToGEMScanType StringToGEMScanType = boost::assign::map_list_of
      ("THRESHOLD", GEMScanType::THRESHOLD)
      ("LATENCY"  , GEMScanType::LATENCY  )
      ("SCURVE"   , GEMScanType::SCURVE   )
      ("HV"       , GEMScanType::HV       );
  }  // namespace gem::base
}  // namespace gem

#endif  // GEM_BASE_GEMSTRINGSTOENUMS_H
