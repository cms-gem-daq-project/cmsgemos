#include "gem/onlinedb/SerializationData.h"

#include <xercesc/dom/DOM.hpp>

#include "gem/onlinedb/exception/Exception.h"
#include "gem/onlinedb/detail/XMLUtils.h"

XERCES_CPP_NAMESPACE_USE

namespace gem {
    namespace onlinedb {
        namespace detail {
            namespace /* anonymous */ {
                using namespace literals;
                using detail::appendChildElement;
                using detail::appendChildText;

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

            std::unique_ptr<DOMXPathResult> queryDataSets(const DOMDocumentPtr &dom)
            {
                // Evaluate query
                // Note: only result types 6 to 9 are supported by Xerces
                auto result = std::unique_ptr<DOMXPathResult>();
                result.reset(dom->evaluate(
                    "//ROOT/DATA_SET"_xml,
                    dom->getDocumentElement(),
                    nullptr,
                    DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                    nullptr));
                return result;
            }

            std::string readDataSetComment(
                const DOMDocumentPtr &document,
                const std::unique_ptr<xercesc::DOMXPathResult> &result)
            {
                auto dataSetElement =
                    dynamic_cast<DOMElement *>(result->getNodeValue());
                auto commentElement = findChildElement(dataSetElement,
                                                       "COMMENT_DESCRIPTION");
                return transcode(commentElement->getTextContent());
            }

            std::string readDataSetVersion(
                const DOMDocumentPtr &document,
                const std::unique_ptr<xercesc::DOMXPathResult> &result)
            {
                auto dataSetElement =
                    dynamic_cast<DOMElement *>(result->getNodeValue());
                auto versionElement = findChildElement(dataSetElement, "VERSION");
                return transcode(versionElement->getTextContent());
            }

            template<>
            PartReferenceBarcode readPartReference<PartReferenceBarcode>(
                const DOMDocumentPtr &document,
                const std::unique_ptr<xercesc::DOMXPathResult> &result)
            {
                auto dataSetElement =
                    dynamic_cast<DOMElement *>(result->getNodeValue());
                auto partElement = findChildElement(dataSetElement, "PART");
                auto barcodeElement = findChildElement(partElement, "BARCODE");
                return PartReferenceBarcode{
                    transcode(barcodeElement->getTextContent())
                };
            }

            template<>
            PartReferenceSN readPartReference<PartReferenceSN>(
                const DOMDocumentPtr &document,
                const std::unique_ptr<xercesc::DOMXPathResult> &result)
            {
                auto dataSetElement =
                    dynamic_cast<DOMElement *>(result->getNodeValue());
                auto partElement = findChildElement(dataSetElement, "PART");
                auto snElement = findChildElement(partElement, "SERIAL_NUMBER");
                return PartReferenceSN{
                    transcode(snElement->getTextContent())
                };
            }

            std::vector<detail::RegisterData> readRegisterData(
                const DOMDocumentPtr &document,
                const std::unique_ptr<xercesc::DOMXPathResult> &result)
            {
                auto dataSetElement =
                    dynamic_cast<DOMElement *>(result->getNodeValue());

                auto vec = std::vector<detail::RegisterData>();

                for (auto dataElement = dataSetElement->getFirstElementChild();
                     dataElement != nullptr;
                     dataElement = dataElement->getNextElementSibling()) {
                    if (detail::transcode(dataElement->getNodeName()) != "DATA") {
                        // Need to skip COMMENT_DESCRIPTION, VERSION and PART
                        continue;
                    }

                    auto data = detail::RegisterData();

                    auto childNodes = dataElement->getChildNodes();
                    auto childCount = childNodes->getLength();

                    // Loop over elements inside DATA tags
                    for (XMLSize_t child = 0; child < childCount; ++child) {
                        auto node = childNodes->item(child);
                        if (node->getNodeType() != DOMNode::ELEMENT_NODE) {
                            continue;
                        }

                        auto name = detail::transcode(node->getNodeName());
                        auto value = detail::transcode(node->getTextContent());

                        data[name] = std::stoi(value);
                    }

                    vec.push_back(data);
                }

                return vec;
            }

            DOMDocumentPtr makeDOM(const std::string &extTableName,
                                   const std::string &comment,
                                   const Run &run)
            {
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
