#ifndef gem_base_utils_GEMInfoSpaceToolBox_h
#define gem_base_utils_GEMInfoSpaceToolBox_h

//using the infospace toolbox defined in the TCDS code base
#include <string>
#include <memory>
#include <unordered_map>

#include "xdata/InfoSpace.h"
#include "xdata/InfoSpaceFactory.h"

#include "xdata/SimpleType.h"
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

#include "gem/base/utils/exception/Exception.h"
#include "gem/utils/GEMLogging.h"

namespace gem {
  
  namespace base {
    class GEMApplication;
    //class GEMMonitor;
    
    namespace utils {
      class GEMInfoSpaceToolBox
      {
      public:
        /**
         * ItemType controls the type of item to store in the info space
         */
        enum ItemType { UINT8,    //!< Used for VFAT registers, 8-bits only
                        UINT16,   //!< Used for certain VFAT registers, 16-bits only, e.g., ChipID
                        UINT24,   //!< Used for certain VFAT registers, 24-bits only, e.g., HitCount
                        UINT32,   //!< Generic 32-bit hardware register
                        UINT64,   //!< Combines two 32-bit registers into a 64-bit value
                        BOOL,     //!< boolean type
                        INTEGER,  //!< integer type
                        DOUBLE,   //!< double type
                        STRING,   //!< string type
                        INFOSPACE //!< embedded InfoSpace type
        };
      
        /**
         * UpdateType controls the type of update to perform on the value stored in the info space
         */
        enum UpdateType { HW8,      //!< Used for VFAT registers, 8-bits only
                          HW16,     //!< Used for certain VFAT registers, 16-bits only, e.g., ChipID
                          HW24,     //!< Used for certain VFAT registers, 24-bits only, e.g., HitCount
                          HW32,     //!< Generic 32-bit hardware register
                          HW64,     //!< Combines two 32-bit registers into a 64-bit value
                          I2CSTAT,  //!< Takes care of registers which have a strobe/acknowlege counter
                          PROCESS,  //!< Copied from Ferol, not sure of the use case
                          TRACKER,  //!< Copied from Ferol, not sure of the use case
                          NOUPDATE  //!< Value is not to be updated
        };
      
        class GEMInfoSpaceItem
        {
          /**
           * Wrapper around a generic GEMInfoSpaceItem
           */
        public:
        GEMInfoSpaceItem(ItemType itype, UpdateType utype, std::string const& itemname, std::string const& docstring, std::string const& format)
          : m_itype(itype),
            m_utype(utype),
            m_name(itemname),
            m_format(format),
            m_docstring(docstring)
            {};
          
          ~GEMInfoSpaceItem() {};
          
          ItemType    m_itype;
          UpdateType  m_utype;
          std::string m_name;
          std::string m_format;
          std::string m_docstring;
        };
      
        /**
         * Constructor from GEMApplication pointer, existing InfoSpace pointer, and GEMMonitor pointer
         * @param gemApp the pointer to the calling application
         * @param infoSpace the pointer to the InfoSpace to be managed
         * @param gemMonitor the pointer to the monitoring application that uses the InfoSpace
         * @param autoPush whether or not to automatically update the changed values in the InfoSpace
         */
        GEMInfoSpaceToolBox(gem::base::GEMApplication* gemApp,
                            xdata::InfoSpace*          infoSpace,
                            //gem::base::GEMMonitor*     gemMonitor,
                            bool                       autoPush);
      
        /**
         * Constructor from GEMApplication pointer, name of InfoSpace to be created, and GEMMonitor pointer
         * @param gemApp the pointer to the calling application
         * @param infoSpaceName the name of the InfoSpace to be created
         * @param gemMonitor the pointer to the monitoring application that uses the InfoSpace
         * @param autoPush whether or not to automatically update the changed values in the InfoSpace
         */
        GEMInfoSpaceToolBox(gem::base::GEMApplication* gemApp,
                            std::string const&         infoSpaceName,
                            //gem::base::GEMMonitor*     gemMonitor,
                            bool                       autoPush);

        ~GEMInfoSpaceToolBox();

        //methods copied from tcds info space helper
        bool createString( std::string const& itemName, std::string const& value,
                           UpdateType type=NOUPDATE, std::string const& docstring="docstring", std::string const& format="");
        bool createBool(   std::string const& itemName, bool        const& value,
                           UpdateType type=NOUPDATE, std::string const& docstring="docstring", std::string const& format="");
        bool createDouble( std::string const& itemName, double      const& value,
                           UpdateType type=NOUPDATE, std::string const& docstring="docstring", std::string const& format="");
        bool createInteger(std::string const& itemName, int         const& value,
                           UpdateType type=HW32, std::string const& docstring="docstring", std::string const& format="dec");
        bool createUInt32( std::string const& itemName, uint32_t    const& value,
                           UpdateType type=HW32, std::string const& docstring="docstring", std::string const& format="hex");
        bool createUInt64( std::string const& itemName, uint64_t    const& value,
                           UpdateType type=HW64, std::string const& docstring="docstring", std::string const& format="hex");
      
        //methods copied from tcds info space helper
        /**
         * Gets the value of the monitored item in the infospace, does not require a toolbox instance
         * @param itemName is the name of the item in the infospace
         */
        std::string getString( std::string const& itemName);
        bool        getBool(   std::string const& itemName);
        double      getDouble( std::string const& itemName);
        int         getInteger(std::string const& itemName);
        uint32_t    getUInt32( std::string const& itemName);
        uint64_t    getUInt64( std::string const& itemName);
      
        //methods copied from tcds info space helper, static, for access without an instance
        /**
         * Gets the value of the monitored item in the infospace, does not require a toolbox instance
         * @param infoSpace is the infospace in which the item is stored
         * @param itemName is the name of the item in the infospace
         */
        static std::string getString( xdata::InfoSpace* infoSpace, std::string const& itemName);
        static bool        getBool(   xdata::InfoSpace* infoSpace, std::string const& itemName);
        static double      getDouble( xdata::InfoSpace* infoSpace, std::string const& itemName);
        static int         getInteger(xdata::InfoSpace* infoSpace, std::string const& itemName);
        static uint32_t    getUInt32( xdata::InfoSpace* infoSpace, std::string const& itemName);
        static uint64_t    getUInt64( xdata::InfoSpace* infoSpace, std::string const& itemName);
      
        //methods copied from tcds info space helper

        /**
         * Sets the value of the monitored item in the infospace
         * @param itemName is the name of the item in the infospace
         * @param value is the value to assign to the item in the infospace
         */
        bool setString( std::string const& itemName, std::string const& value);
        bool setBool(   std::string const& itemName, bool        const& value);
        bool setDouble( std::string const& itemName, double      const& value);
        bool setInteger(std::string const& itemName, int         const& value);
        bool setUInt32( std::string const& itemName, uint32_t    const& value);
        bool setUInt64( std::string const& itemName, uint64_t    const& value);

        xdata::InfoSpace* getInfoSpace()  { return p_infoSpace;         };
        std::string       name()          { return p_infoSpace->name(); };
        bool find(std::string const& key) { return m_itemMap.find(key) != m_itemMap.end(); };

        /**
         * Way to display items in a formatted way
         * @param itemName is the name of the item in the info space
         * @param format is the format that the value should be displayed as
         * @returns the item from the infospace according to the type and format specified
         */
        std::string getFormattedItem(std::string const& itemName, std::string const& format);
        
        /**
         * Print the docstring associated with the infospace item
         * @param itemName is the name of the item in the info space
         * @returns the docstring associated with the info space item
         */
        std::string getItemDocstring(std::string const& itemName);
        
        /**
         * Takes care of cleaning up the infospace after a reset
         * should empty all lists and maps of known items
         */
        void reset();
        
      private:
        log4cplus::Logger m_gemLogger;

        std::unordered_map<std::string, GEMInfoSpaceItem*> m_itemMap;

        // maps of variables and their associated xdata types
        std::unordered_map<std::string, std::pair<uint32_t,    xdata::UnsignedInteger32* > >  m_uint32Items;
        std::unordered_map<std::string, std::pair<uint64_t,    xdata::UnsignedInteger64* > >  m_uint64Items;
        std::unordered_map<std::string, std::pair<bool,        xdata::Boolean* > >            m_boolItems;
        std::unordered_map<std::string, std::pair<int,         xdata::Integer* > >            m_intItems;
        std::unordered_map<std::string, std::pair<double,      xdata::Double*  > >            m_doubleItems;
        std::unordered_map<std::string, std::pair<std::string, xdata::String*  > >            m_stringItems;

      protected:
        gem::base::GEMApplication* p_gemApp;
        //gem::base::GEMMonitor*     p_gemMonitor;
        //std::shared_ptr<gem::base::GEMApplication> p_gemApp;
        ////std::shared_ptr<gem::base::GEMMonitor>     p_gemMonitor;
        xdata::InfoSpace* p_infoSpace;
      };
    
    }//end namespace gem::base::utils
  }//end namespace gem::base
}//end namespace gem

#endif 
