#include "gem/onlinedb/detail/XMLUtils.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "gem/onlinedb/exception/Exception.h"

namespace gem {
    namespace onlinedb {
        namespace detail {

            std::string transcode(const XMLCh * const xercesStr)
            {
                char *transcoded = xercesc::XMLString::transcode(xercesStr);
                std::string result(transcoded);
                xercesc::XMLString::release(&transcoded);
                return result;
            }

            std::string transcodeAndRelease(XMLCh *xercesStr)
            {
                std::string transcoded = transcode(xercesStr);
                xercesc::XMLString::release(&xercesStr);
                return transcoded;
            }

            XercesGuard::XercesGuard()
            {
                try {
                    // Initialize Xerces
                    xercesc::XMLPlatformUtils::Initialize();
                } catch (const xercesc::XMLException &e) {
                    XCEPT_RAISE(exception::SoftwareProblem,
                                std::string("Xerces failed to initialize: ") +
                                    detail::transcode(e.getMessage()));
                }
            }

            XercesGuard::~XercesGuard()
            {
                xercesc::XMLPlatformUtils::Terminate();
            }

            xercesc::DOMElement *appendChildElement(
                xercesc::DOMElement *parent, const std::string &tagName)
            {
                auto xmlTagName = detail::XercesString(tagName);
                auto element = parent->getOwnerDocument()->createElement(xmlTagName);
                XCEPT_ASSERT(element != nullptr, exception::SoftwareProblem,
                             "Xerces failed to create element " + tagName);
                parent->appendChild(element);
                return element;
            }

            xercesc::DOMText *appendChildText(
                xercesc::DOMElement *parent, const std::string &content)
            {
                auto xmlContent = detail::XercesString(content);
                auto node = parent->getOwnerDocument()->createTextNode(xmlContent);
                XCEPT_ASSERT(node != nullptr, exception::SoftwareProblem,
                             "Xerces failed to create text node");
                parent->appendChild(node);
                return node;
            }

        } /* namespace detail */
    } /* namespace onlinedb */
} /* namespace gem */
