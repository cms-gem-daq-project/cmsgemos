#include "gem/onlinedb/detail/XMLUtils.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "gem/onlinedb/exception/Exception.h"

namespace gem {
    namespace onlinedb {
        namespace detail {
            using namespace literals;

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

            xercesc::DOMNode *xsdGet(const DOMDocumentPtr &document,
                                     const char *query,
                                     const xercesc::DOMNode *root)
            {
                if (root == nullptr) {
                    root = document->getDocumentElement();
                }

                // Create a name resolver
                auto nsResolver = std::unique_ptr<xercesc::DOMXPathNSResolver>();
                nsResolver.reset(document->createNSResolver(root));

                // Evaluate query
                // Note: only result types 6 to 9 are supported by Xerces
                auto result = std::unique_ptr<xercesc::DOMXPathResult>();
                result.reset(document->evaluate(
                    detail::XercesString(query),
                    root,
                    nsResolver.get(),
                    xercesc::DOMXPathResult::FIRST_ORDERED_NODE_TYPE,
                    nullptr));

                // Get resulting DOMNode
                return result->getNodeValue();
            }

            std::string xsdGetTextContent(const DOMDocumentPtr &document,
                                          const char *query,
                                          const xercesc::DOMNode *root)
            {
                // Get node
                auto node = xsdGet(document, query, root);
                return detail::transcode(node->getTextContent());
            }

        } /* namespace detail */
    } /* namespace onlinedb */
} /* namespace gem */
