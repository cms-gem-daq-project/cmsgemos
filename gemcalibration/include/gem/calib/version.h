/** @file version.h */

#ifndef GEM_CALIB_VERSION_H
#define GEM_CALIB_VERSION_H

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

namespace gemcalibration {


#define GEMCALIBRATION_VERSION_MAJOR 0
#define GEMCALIBRATION_VERSION_MINOR 0
#define GEMCALIBRATION_VERSION_PATCH 0
#define GEMCALIBRATION_PREVIOUS_VERSIONS ""

#define GEMCALIBRATION_VERSION_CODE PACKAGE_VERSION_CODE(GEMCALIBRATION_VERSION_MAJOR, GEMCALIBRATION_VERSION_MINOR, GEMCALIBRATION_VERSION_PATCH)

#ifndef GEMCALIBRATION_PREVIOUS_VERSIONS
#define GEMCALIBRATION_FULL_VERSION_LIST  PACKAGE_VERSION_STRING(GEMCALIBRATION_VERSION_MAJOR, GEMCALIBRATION_VERSION_MINOR, GEMCALIBRATION_VERSION_PATCH)
#else
#define GEMCALIBRATION_FULL_VERSION_LIST  GEMCALIBRATION_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMCALIBRATION_VERSION_MAJOR, GEMCALIBRATION_VERSION_MINOR, GEMCALIBRATION_VERSION_PATCH)
#endif

      const std::string project     = "cmsgemos";
      const std::string package     = "gemcalibration";
      const std::string versions    = GEMCALIBRATION_FULL_VERSION_LIST;
      const std::string summary     = "Calibration suite";
      const std::string description = "";
      const std::string authors     = "GEM Online Systems Group";
      const std::string link        = "https://cms-gem-daq-project.github.io/cmsgemos/";

      config::PackageInfo getPackageInfo();
      void checkPackageDependencies();
      std::set<std::string,std::less<std::string> > getPackageDependencies();
  
}

#endif // DOXYGEN_IGNORE_THIS

#endif // GEM_CALIB_VERSION_H
