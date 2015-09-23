#ifndef gem_utils_GEMInfoSpaceToolBox_h
#define gem_utils_GEMInfoSpaceToolBox_h

//using the infospace toolbox defined in the TCDS code base
#include <string>

#include "xdata/InfoSpace.h"

#include "xdata/Boolean.h"
#include "xdata/Integer.h"
#include "xdata/Integer32.h"
#include "xdata/Integer64.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/UnsignedInteger64.h"
#include "xdata/String.h"
#include "xdata/Float.h" 
#include "xdata/Double.h" 
#include "xdata/Serializable.h"

#include "gem/utils/exception/Exception.h"

namespace gem {
  namespace utils {
    
    class GEMInfoSpaceToolBox
    {
    public:
      //methods copied from tcds info space helper
      static std::string getString(xdata::InfoSpace* infoSpace,
                                   std::string const& itemName);
      static bool        getBool(xdata::InfoSpace* infoSpace,
                                 std::string const& itemName);
      static double      getDouble(xdata::InfoSpace* infoSpace,
                                   std::string const& itemName);
      static int         getInteger(xdata::InfoSpace* infoSpace,
                                    std::string const& itemName);
      static uint32_t    getUInt32(xdata::InfoSpace* infoSpace,
                                   std::string const& itemName);
      static uint64_t    getUInt64(xdata::InfoSpace* infoSpace,
                                   std::string const& itemName);

      //methods copied from tcds info space helper
      static bool setString(xdata::InfoSpace* infoSpace,
                            std::string const& itemName,
                            std::string const& value);
      static bool setBool(xdata::InfoSpace* infoSpace,
                          std::string const& itemName,
                          bool const& value);
      static bool setDouble(xdata::InfoSpace* infoSpace,
                            std::string const& itemName,
                            double const& value);
      static bool setInteger(xdata::InfoSpace* infoSpace,
                             std::string const& itemName,
                             int const& value);
      static bool setUInt32(xdata::InfoSpace* infoSpace,
                            std::string const& itemName,
                            uint32_t const& value);
      static bool setUInt64(xdata::InfoSpace* infoSpace,
                            std::string const& itemName,
                            uint64_t const& value);
    private:
      
    protected:
      GEMInfoSpaceToolBox();
    };
    
  }//end namespace gem::base

}//end namespace gem

#endif 
