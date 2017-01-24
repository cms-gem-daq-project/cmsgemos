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

namespace gem {
    namespace utils {
        namespace vfat {
            class VFAT2ConfigManager
            {

                public:
                    VFAT2ConfigManager(VFAT2ControlParams inParams);
                    VFAT2ConfigManager(const std::string& glxmlFile, const std::string& chxmlFile);

                    ~VFAT2ConfigManager();

                    void parseXMLFiles();

                    /**
                     *   Parse Header in Global XML file
                     */
                    void parseGLheader(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Header in Channel XML file
                     */
                    void parseCHheader(xercesc::DOMNode * pNode);
                    /**
                     *   Parse Dataset node in Global XML file
                     */
                    void parseGLdataset(xercesc::DOMNode * pNode);
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
                    static void addProperty(const char* key, const xercesc::DOMNode* n, gemVFATProperties* vfat);

                    VFAT2ControlParams localParams;

                    std::string m_glxmlFile;
                    std::string m_chxmlFile;
                    gemSystemProperties *p_gemSystem;
                    log4cplus::Logger m_gemLogger;
            };
        }
    }  // end namespace gem::utils
}  // end namespace gem

#endif  // GEM_UTILS_VFAT2CONFIGMANAGER_
