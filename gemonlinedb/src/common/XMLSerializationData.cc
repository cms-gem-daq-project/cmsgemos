#include "gem/onlinedb/XMLSerializationData.h"

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
                 * @brief Executes an XSD query on the given document and
                 *        retrieves the corresponding DOMNode.
                 */
                DOMNode *xsdGet(const DOMDocumentPtr &document,
                                const char *query,
                                const DOMNode *root = nullptr)
                {
                    if (root == nullptr) {
                        root = document->getDocumentElement();
                    }

                    // Transcode query
                    auto transcoded = XMLString::transcode(query);

                    // Evaluate query
                    // Note: only result types 6 to 9 are supported by Xerces
                    auto result = document->evaluate(
                        transcoded,
                        root,
                        nullptr,
                        DOMXPathResult::FIRST_ORDERED_NODE_TYPE,
                        nullptr);

                    // Get resulting DOMNode
                    auto node = result->getNodeValue();

                    // Cleanup
                    XMLString::release(&transcoded);
                    delete result;

                    return node;
                }

                /**
                 * @brief Executes an XSD query on the given document and
                 *        retrieves the text content of the corresponding
                 *        DOMNode.
                 */
                std::string xsdGetTextContent(const DOMDocumentPtr &document,
                                              const char *query,
                                              const DOMNode *root = nullptr)
                {
                    // Get node
                    auto node = xsdGet(document, query, root);

                    // Get its text content
                    auto utf16 = node->getTextContent();

                    // Transcode
                    auto transcoded = XMLString::transcode(utf16);

                    // Turn into std::string with proper memory management
                    std::string result(transcoded);

                    // Cleanup and return
                    delete transcoded;
                    return result;
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

            Run getRun(const DOMDocumentPtr &dom)
            {
                Run r;
                r.type = xsdGetTextContent(dom, "//ROOT/HEADER/RUN/RUN_TYPE");
                r.number =
                    std::stoll(xsdGetTextContent(dom, "//ROOT/HEADER/RUN/RUN_NUMBER"));
                r.begin = xsdGetTextContent(dom, "//ROOT/HEADER/RUN/RUN_BEGIN_TIMESTAMP");
                r.end = xsdGetTextContent(dom, "//ROOT/HEADER/RUN/RUN_END_TIMESTAMP");
                r.description =
                    xsdGetTextContent(dom, "//ROOT/HEADER/RUN/COMMENT_DESCRIPTION");
                r.location = xsdGetTextContent(dom, "//ROOT/HEADER/RUN/LOCATION");
                r.initiatingUser =
                    xsdGetTextContent(dom, "//ROOT/HEADER/RUN/INITIATED_BY_USER");
                return r;
            }

            DOMXPathResult *queryDataSets(const DOMDocumentPtr &dom)
            {
                // Evaluate query
                // Note: only result types 6 to 9 are supported by Xerces
                return dom->evaluate(
                    "//ROOT/DATA_SET"_xml,
                    dom->getDocumentElement(),
                    nullptr,
                    DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                    nullptr);
            }

            std::string readDataSetComment(const DOMDocumentPtr &document,
                                           const DOMXPathResult *result)
            {
                return xsdGetTextContent(document,
                                         "//DATA_SET/COMMENT_DESCRIPTION",
                                         result->getNodeValue());
            }

            std::string readDataSetVersion(const DOMDocumentPtr &document,
                                           const DOMXPathResult *result)
            {
                return xsdGetTextContent(document,
                                         "//DATA_SET/VERSION",
                                         result->getNodeValue());
            }

            template<>
            PartReferenceBarcode readPartReference<PartReferenceBarcode>(
                const DOMDocumentPtr &document,
                const DOMXPathResult *result)
            {
                PartReferenceBarcode ref;
                ref.barcode = xsdGetTextContent(document,
                                                "//DATA_SET/PART/BARCODE",
                                                result->getNodeValue());
                return ref;
            }

            template<>
            PartReferenceSN readPartReference<PartReferenceSN>(
                const DOMDocumentPtr &document,
                const DOMXPathResult *result)
            {
                PartReferenceSN ref;
                ref.serialNumber = xsdGetTextContent(
                    document,
                    "//DATA_SET/PART/SERIAL_NUMBER",
                    result->getNodeValue());
                return ref;
            }

            std::vector<detail::RegisterData> readRegisterData(
                const DOMDocumentPtr &document,
                const xercesc::DOMXPathResult *result)
            {
                // Evaluate query
                // Note: only result types 6 to 9 are supported by Xerces
                auto queryResult = document->evaluate(
                    "//DATA_SET/DATA"_xml,
                    result->getNodeValue(),
                    nullptr,
                    DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                    nullptr);

                auto count = queryResult->getSnapshotLength();
                auto vec = std::vector<detail::RegisterData>();
                vec.reserve(count);

                // Loop over DATA tags
                for (XMLSize_t i = 0; i < count; ++i) {
                    queryResult->snapshotItem(i);

                    auto data = detail::RegisterData();

                    auto childNodes = queryResult->getNodeValue()->getChildNodes();
                    auto childCount = childNodes->getLength();

                    // Loop over elements inside DATA tags
                    for (XMLSize_t child = 0; child < childCount; ++child) {
                        auto node = childNodes->item(child);

                        auto nameUtf16 = node->getNodeName();
                        auto valueUtf16 = node->getTextContent();

                        auto name = XMLString::transcode(nameUtf16);
                        auto value = XMLString::transcode(valueUtf16);

                        data[name] = std::stoi(value);

                        XMLString::release(&name);
                        XMLString::release(&value);
                    }

                    vec.push_back(data);
                }

                return vec;
            }

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
