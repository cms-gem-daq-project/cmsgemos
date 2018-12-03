#include "gem/onlinedb/XMLBuilder.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

#include <stdexcept>

#include <iomanip>
#include <iostream>

#include "gem/onlinedb/exception/Exception.h"

XERCES_CPP_NAMESPACE_USE

namespace gem {
    namespace onlinedb {
        namespace detail {
            namespace /* anonymous */ {
                /**
                 * @brief Convenience string literal to shorten Xerces code.
                 */
                const XMLCh *operator"" _xml (const char *string, std::size_t)
                {
                    return XMLString::transcode(string);
                }

                /**
                 * @brief Appends a child element to @c parent with tag name
                 *        @c tagName.
                 *
                 * The new element is created without any attribute or child.
                 * @returns The new element.
                 */
                DOMElement *appendChildElement(DOMElement *parent,
                                               const std::string &tagName)
                {
                    auto xmlTagName = XMLString::transcode(tagName.c_str());
                    auto element = parent->getOwnerDocument()->createElement(xmlTagName);
                    XCEPT_ASSERT(element != nullptr, exception::SoftwareProblem,
                                 "Xerces failed to create element " + tagName);
                    parent->appendChild(element);
                    return element;
                }

                /**
                 * @brief Appends a child text element to @c parent, with given
                 *        @c contents.
                 * @returns The new text node.
                 */
                DOMText *appendChildText(DOMElement *parent,
                                         const std::string &content)
                {
                    auto xmlContent = XMLString::transcode(content.c_str());
                    auto node = parent->getOwnerDocument()->createTextNode(xmlContent);
                    XCEPT_ASSERT(node != nullptr, exception::SoftwareProblem,
                                 "Xerces failed to create text node");
                    parent->appendChild(node);
                    return node;
                }

                /**
                 * @brief Creates a RUN element
                 */
                void makeRunElement(DOMElement *parent, const Run &run)
                {
                    auto runElement = appendChildElement(parent, "RUN");

                    auto type = appendChildElement(runElement, "RUN_TYPE");
                    appendChildText(type, run.type);

                    auto number = appendChildElement(runElement, "RUN_NUMBER");
                    appendChildText(number, std::to_string(run.number));

                    auto begin = appendChildElement(runElement, "RUN_BEGIN_TIMESTAMP");
                    appendChildText(begin, run.begin);

                    auto end = appendChildElement(runElement, "RUN_END_TIMESTAMP");
                    appendChildText(end, run.end);

                    auto comment = appendChildElement(runElement, "COMMENT_DESCRIPTION");
                    appendChildText(comment, run.description);

                    auto location = appendChildElement(runElement, "LOCATION");
                    appendChildText(location, run.location);

                    auto user = appendChildElement(runElement, "INITIATED_BY_USER");
                    appendChildText(user, run.initiatingUser);
                }
            } /* anonymous namespace */

            DOMDocumentPtr makeDOM(const std::string &extTableName,
                                   const std::string &comment,
                                   const Run &run)
            {
                try {
                    // Initialize Xerces
                    XMLPlatformUtils::Initialize();
                } catch (const XMLException &e) {
                    XCEPT_RAISE(exception::SoftwareProblem,
                                std::string("Xerces failed to initialize: ") +
                                    XMLString::transcode(e.getMessage()));
                }

                // Get the implementation
                auto impl = DOMImplementationRegistry::getDOMImplementation("LS"_xml);
                XCEPT_ASSERT(impl != nullptr, exception::SoftwareProblem,
                             "Xerces couldn't find a DOM implementation");

                // Create the document
                auto document = DOMDocumentPtr();
                document.reset(impl->createDocument(nullptr, "ROOT"_xml, nullptr));
                XCEPT_ASSERT(document != nullptr, exception::SoftwareProblem,
                             "Xerces couldn't create a document");
                auto docRoot = document->getDocumentElement();

                // Header
                auto header = appendChildElement(docRoot, "HEADER");

                auto type = appendChildElement(header, "TYPE");
                auto extTableNameElement = appendChildElement(type,
                                                              "EXTENSION_TABLE_NAME");
                appendChildText(extTableNameElement, extTableName);
                auto name = appendChildElement(type, "NAME");
                appendChildText(name, comment);

                makeRunElement(header, run);

                return document;
            }

            DOMElement *createDataSetElement(DOMDocumentPtr &document,
                                             const std::string &comment,
                                             const std::string &version)
            {
                // DATA_SET elements are located under the root
                auto root = document->getDocumentElement();
                auto dataSet = appendChildElement(root, "DATA_SET");

                auto commentElement = appendChildElement(dataSet, "COMMENT_DESCRIPTION");
                appendChildText(commentElement, comment);

                auto versionElement = appendChildElement(dataSet, "VERSION");
                appendChildText(versionElement, version);

                return dataSet;
            }

            void createPartElement(DOMElement *parent,
                                   const std::string &kindOfPart,
                                   const PartReferenceBarcode &partRef)
            {
                auto part = appendChildElement(parent, "PART");

                auto kindOfPartElement = appendChildElement(part, "KIND_OF_PART");
                appendChildText(kindOfPartElement, kindOfPart);

                auto barcode = appendChildElement(part, "BARCODE");
                appendChildText(barcode, partRef.barcode);
            }

            void createPartElement(DOMElement *parent,
                                   const std::string &kindOfPart,
                                   const PartReferenceSN &partRef)
            {
                auto part = appendChildElement(parent, "PART");

                auto kindOfPartElement = appendChildElement(part, "KIND_OF_PART");
                appendChildText(kindOfPartElement, kindOfPart);

                auto barcode = appendChildElement(part, "SERIAL_NUMBER");
                appendChildText(barcode, partRef.serialNumber);
            }

            void createDataElement(DOMElement *parent,
                                   const RegisterData &content)
            {
                auto data = appendChildElement(parent, "DATA");

                for (const auto &entry: content) {
                    auto element = appendChildElement(data, entry.first);
                    appendChildText(element, std::to_string(entry.second));
                }
            }
        } /* namespace detail */
    } /* namespace onlinedb */
} /* namespace gem */
