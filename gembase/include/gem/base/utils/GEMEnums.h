/** @file GEMEnums.h */

#ifndef GEM_BASE_GEMENUMS_H
#define GEM_BASE_GEMENUMS_H

namespace gem {
  namespace base {

    struct GEMRunType {
      enum EGEMRunType {
        DATA      = 0x1,
        COSMIC    = 0x2,
        TESTSTAND = 0x3,
        SCAN      = 0x4,
      } GEMRunType;
    };

    struct GEMScanType {
      enum EGEMScanType {
        LATENCY   = 0x2,
        THRESHOLD = 0x3,
        SCURVE    = 0x4,
        HV        = 0x5
      } GEMScanType;
    };

  }  // namespace gem::base
}  // namespace gem

#endif  // GEM_BASE_GEMENUMS_H
