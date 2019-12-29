#include "gem/onlinedb/AMC13Configuration.h"

#include <sstream>

#include <xercesc/dom/DOMXPathResult.hpp>

#include "gem/onlinedb/SerializationData.h"
#include "gem/onlinedb/detail/XMLUtils.h"
#include "gem/onlinedb/exception/Exception.h"

XERCES_CPP_NAMESPACE_USE

namespace gem {
    namespace onlinedb {

        void SerializationData<AMC13Configuration>::readDOM(
            const DOMDocumentPtr &dom)
        {
            using namespace detail::literals;

            using Traits = ConfigurationTraits<ConfigurationType>;
            using PartType = typename Traits::PartType;

            // Get run info
            m_run = detail::getRun(dom);

            // Iterate over DATA_SETs
            auto dataSetsResult = detail::queryDataSets(dom);
            auto count = dataSetsResult->getSnapshotLength();
            for (XMLSize_t i = 0; i < count; ++i) {
                dataSetsResult->snapshotItem(i);

                m_dataSets.emplace_back();
                auto &dataSet = m_dataSets.back();

                dataSet.setComment(detail::readDataSetComment(dom, dataSetsResult));
                dataSet.setVersion(detail::readDataSetVersion(dom, dataSetsResult));
                dataSet.setPart(
                    detail::readPartReference<PartType>(dom, dataSetsResult));

                auto dataSetElement =
                    dynamic_cast<DOMElement *>(dataSetsResult->getNodeValue());

                // Loop over DATA tags
                // We assume that their contents are well-formed. The user is
                // responsible for checking this using the schema.
                for (auto dataElement = dataSetElement->getFirstElementChild();
                     dataElement != nullptr;
                     dataElement = dataElement->getNextElementSibling()) {
                    if (detail::transcode(dataElement->getNodeName()) != "DATA") {
                        // Need to skip COMMENT_DESCRIPTION, VERSION and PART
                        continue;
                    }

                    auto FEDIdNode = detail::findChildElement(dataElement, "FED_ID");
                    auto TTCNode =
                        detail::findChildElement(dataElement, "ENABLE_LOCALTTC");
                    auto hostnameNode =
                        detail::findChildElement(dataElement, "HOSTNAME");

                    auto config = AMC13Configuration();
                    config.setFEDId(
                        std::stoi(detail::transcode(FEDIdNode->getTextContent())));
                    config.setEnableLocalTTC(
                        std::stoi(detail::transcode(TTCNode->getTextContent())));
                    config.setHostname(
                        detail::transcode(hostnameNode->getTextContent()));

                    dataSet.addData(config);
                }
            }
        }

        DOMDocumentPtr SerializationData<AMC13Configuration>::makeDOM() const
        {
            using Info = ConfigurationTraits<ConfigurationType>;

            // Create the general DOM structure
            auto dom = detail::makeDOM(Info::extTableName(), Info::typeName(), m_run);

            for (const auto &dataSet : m_dataSets) {
                // Add a DATA_SET node
                auto dataSetNode = detail::createDataSetElement(dom,
                                                                dataSet.getComment(),
                                                                dataSet.getVersion());
                // Create the PART element
                detail::createPartElement(dataSetNode,
                                          Info::kindOfPart(),
                                          dataSet.getPart());
                // Add DATA elements
                for (const auto &entry : dataSet.getData()) {
                    using detail::appendChildElement;
                    using detail::appendChildText;

                    auto data = appendChildElement(dataSetNode, "DATA");

                    auto FEDIdElement = appendChildElement(data, "FED_ID");
                    appendChildText(FEDIdElement,
                                    std::to_string(entry.getFEDId()));

                    auto localTTCElement = appendChildElement(data, "ENABLE_LOCALTTC");
                    appendChildText(localTTCElement,
                                    std::to_string((int) entry.getEnableLocalTTC()));

                    auto hostnameElement = appendChildElement(data, "HOSTNAME");
                    appendChildText(hostnameElement, entry.getHostname());
                }
            }
            return dom;
        }

        template<>
        AMC13Configuration DBInterface::convertRow<AMC13Configuration>(
            const xdata::Table &table, xdata::Table::Row &row)
        {
            AMC13Configuration config;

            config.setFEDId(toInt(row, "FED_ID"));
            config.setEnableLocalTTC(toInt(row, "ENABLE_LOCALTTC"));

            auto value = row.getField("HOSTNAME");
            if (value == nullptr) {
                XCEPT_RAISE(exception::ParseError,
                            "In database field \"HOSTNAME\": Null value");
            } else if (value->type() == "string") {
                config.setHostname(value->toString());
            } else {
                XCEPT_RAISE(exception::ParseError,
                            "In database field \"HOSTNAME\": Unknown type " +
                            value->type());
            }

            return config;
        }
    } /* namespace onlinedb */
} /* namespace gem */
