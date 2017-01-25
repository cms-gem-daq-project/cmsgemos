/** @file VFAT2ConfigManager.h */

#ifndef GEM_UTILS_VFAT2CONFIGMANAGER_H
#define GEM_UTILS_VFAT2CONFIGMANAGER_H

#include <map>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>
#include <xercesc/dom/DOMLSOutput.hpp>

#include <gem/utils/GEMLogging.h>
#include <gem/utils/gemComplexDeviceProperties.h>

#include <gem/hw/vfat/VFAT2Settings.h>
#include <gem/hw/vfat/VFAT2SettingsEnums.h>
#include <gem/hw/vfat/VFAT2Strings2Enums.h>

namespace gem {
    namespace utils {
        namespace vfat {
            class VFAT2ConfigManager
            {

                public:
                    //VFAT2ConfigManager(VFAT2ControlParams inParams);
                    VFAT2ConfigManager(const std::string& glxmlFile, const std::string& chxmlFile);

                    ~VFAT2ConfigManager();

                    void parseXMLFiles();

                    /**
                     *   Set Path to Global XML file
                     */
                    void setGLfile(const std::string& glxmlFile);
                    /**
                     *   Set Path to Channel XML file
                     */
                    void setCHfile(const std::string& chxmlFile);
                    /**
                     *   Parse Header in Global XML file
                     */
                    void parseGLheader(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Header in Channel XML file
                     */
                    void parseCHheader(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Header Type in Global XML file
                     */
                    void parseGLheaderType(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Header Type in Channel XML file
                     */
                    void parseCHheaderType(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Header Run in Global XML file
                     */
                    void parseGLheaderRun(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Header Run in Channel XML file
                     */
                    void parseCHheaderRun(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Dataset node in Global XML file
                     */
                    void parseGLdataset(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Dataset-Part node in Global XML file
                     */
                    void parseGLdatasetPart(xercesc::DOMNode * pNode);
                    /**
                     *   Parse part node in Global XML file
                     */
                    void parseGLpart(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Dataset node in Channel XML file
                     */
                    void parseCHdataset(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Data node in Global XML file
                     */
                    void parseGLdata(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Data node in Channel XML file
                     */
                    void parseCHdata(xercesc::DOMNode * pNode);

                    int countChildElementNodes(xercesc::DOMNode * pNode);
                    void outputXML(xercesc::DOMDocument* pmyDOMDocument, std::string filePath);

                private:
                    struct Vheader
                    {
                        struct Type
                        {
                            std::string ETN;
                            std::string name;
                        };

                        struct Run
                        {
                            std::string runType;
                            std::string runNumber;
                            std::string runBtime;
                            std::string runEtime;
                            std::string comment;
                            std::string location;
                            std::string user;
                        };

                        Type type;
                        Run run;
                    };

                    struct metaData
                    {
                        std::string comment;
                        std::string version;
                        std::string partType;
                        std::string serialN;
                    };

                    Vheader vhead;
                    metaData metadata;

                    gem::hw::vfat::VFAT2ControlParams localParams;

                    std::string m_glxmlFile;
                    std::string m_chxmlFile;
                    //gemSystemProperties *p_gemSystem;
                    log4cplus::Logger m_gemLogger;
            };
        }
    }  // end namespace gem::utils
}  // end namespace gem

#endif  // GEM_UTILS_VFAT2CONFIGMANAGER_
