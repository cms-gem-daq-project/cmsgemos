#ifndef gem_hw_vfat_GEMXMLParser_h
#define gem_hw_vfat_GEMXMLParser_h

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

            class GEMXMLParser
            {

                public:
                    GEMXMLParser(std::string& xmlFile, gem::hw::vfat::HwVFAT2* vfatDevice);

                    ~GEMXMLParser();

                    void parseXMLFile();

                    /** 
                     *   Parse section of XML configuration file describing GEM system
                     */
                    void parseGEMSystem(xercesc::DOMNode * pNode);

                    /** 
                     *   Parse section of XML configuration file describing available uTCA crates
                     */
                    void parseCrate(xercesc::DOMNode * pNode);

                    /** 
                     *   Parse section of XML configuration file describing available GLIB board(s)
                     */
                    void parseGLIB(xercesc::DOMNode * pNode);

                    /** 
                     *   Parse section of XML configuration file describing available OH board(s)
                     */
                    void parseOH(xercesc::DOMNode * pNode);

                    /** 
                     *   Parse section of XML configuration file describing VFAT2 registers
                     */
                    void parseVFAT2Settings(xercesc::DOMNode * pNode);

                    /**
                     *   Parse section of XML configuration file describing VFAT2 control registers
                     */
                    void parseControlRegisters(xercesc::DOMNode * pNode);

                    /**
                     *   Parse section of XML configuration file describing VFAT2 control register 0
                     */
                    void parseControlRegister0(xercesc::DOMNode * pNode);

                    /**
                     *   Parse section of XML configuration file describing VFAT2 control register 1
                     */
                    void parseControlRegister1(xercesc::DOMNode * pNode);

                    /**
                     *   Parse section of XML configuration file describing VFAT2 control register 2
                     */
                    void parseControlRegister2(xercesc::DOMNode * pNode);

                    /**
                     *   Parse section of XML configuration file describing VFAT2 control register 3
                     */
                    void parseControlRegister3(xercesc::DOMNode * pNode);

                    /**
                     *   Parse section of XML configuration file describing VFAT2 bias settings
                     */
                    void parseBiasSettings(xercesc::DOMNode * pNode);

                    /**
                     *   Parse section of XML configuration file describing VFAT2 current bias settings
                     */
                    void parseCurrentBias(xercesc::DOMNode * pNode);

                private:
                    std::string xmlFile_;
                    gem::hw::vfat::HwVFAT2* vfatDevice_;
            };
        }
    }
}

#endif

