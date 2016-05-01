#ifndef GEM_SUPERVISOR_TBUTILS_VFAT2XMLPARSER_H
#define GEM_SUPERVISOR_TBUTILS_VFAT2XMLPARSER_H

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

    }  // namespace gem::supervisor::tbutils
  }  // namespace gem::supervisor
}  // namespace gem

#endif  // GEM_SUPERVISOR_TBUTILS_VFAT2XMLPARSER_H
