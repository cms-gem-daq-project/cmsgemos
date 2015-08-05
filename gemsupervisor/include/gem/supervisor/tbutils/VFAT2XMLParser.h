#ifndef gem_supervisor_tbutils_VFAT2XMLParser_h
#define gem_supervisor_tbutils_VFAT2XMLParser_h

#include <string>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

namespace gem {
  namespace hw {
    namespace vfat {
      class HwVFAT2;
    }
  }
	    
  namespace supervisor {
    namespace tbutils {
      
      class VFAT2XMLParser
      {
	
      public:
        VFAT2XMLParser(const std::string& xmlFile,
                       gem::hw::vfat::HwVFAT2* vfatDevice);

        ~VFAT2XMLParser();

        void parseXMLFile();
        void parseTURBO(xercesc::DOMNode * pNode);
        void parseVFAT(xercesc::DOMNode * pNode);
      private:
        std::string xmlFile_;
        gem::hw::vfat::HwVFAT2* vfatDevice_;
      };
      
    }//end namespace gem::supervisor::tbutils
    
  }//end namespace gem::supervisor
  
}//end namespace gem

#endif
  
