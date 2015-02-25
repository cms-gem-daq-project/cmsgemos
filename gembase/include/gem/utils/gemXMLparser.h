#ifndef gem_base_utils_gemXMLparser_h
#define gem_base_utils_gemXMLparser_h

#include <string>
#include <map>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

namespace gem {
    namespace base {
        namespace utils {
            //class gemCrateProperties;
            class gemDeviceProperties;
            class gemXMLparser
            {

                public:
                    gemXMLparser(const std::string& xmlFile);

                    ~gemXMLparser();

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

                    //const std::vector<gemCrateProperties*>& getCrateRefs() {return crateRefs_;}
                    const gemDeviceProperties & getGEMDevice() {return gemDevice_;}

                private:
                    std::string xmlFile_;
                    gemDeviceProperties *gemDevice_;
                    //std::vector <gemCrateProperties*> crateRefs_;
            };
        }
    }
}

#endif
