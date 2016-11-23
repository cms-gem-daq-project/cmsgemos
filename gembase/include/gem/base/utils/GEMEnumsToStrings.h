/** @file GEMStringsToEnums.h */

#ifndef GEM_BASE_GEMENUMSTOSTRINGS_H
#define GEM_BASE_GEMENUMSTOSTRINGS_H

#include "gem/base/GEMEnums.h"

#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>

namespace gem {
  namespace base {
    typedef boost::unordered_map<GEMRunType::EGEMRunType, std::string>   enumGEMRunTypeToString;
    typedef boost::unordered_map<GEMScanType::EGEMScanType, std::string> enumGEMScanTypeToString;

    const enumGEMRunTypeToString GEMRunTypeToString = boost::assign::map_list_of
      (GEMRunType::DATA     , "DATA"     )
      (GEMRunType::COSMIC   , "COSMIC"   )
      (GEMRunType::TESTSTAND, "TESTSTAND")
      (GEMRunType::SCAN     , "SCAN"     );

    const enumGEMScanTypeToString GEMScanTypeToString = boost::assign::map_list_of
      (GEMScanType::THRESHOLD, "THRESHOLD")
      (GEMScanType::LATENCY  , "LATENCY"  )
      (GEMScanType::SCURVE   , "SCURVE"   )
      (GEMScanType::HV       , "HV"       );
  }  // namespace gem::base
}  // namespace gem

#endif  // GEM_BASE_GEMENUMSTOSTRINGS_H
