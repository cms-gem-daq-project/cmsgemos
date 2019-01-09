#include "gem/onlinedb/AMC13Configuration.h"

#include <sstream>

#include <xercesc/dom/DOMXPathResult.hpp>

#include "gem/onlinedb/XMLSerializationData.h"
#include "gem/onlinedb/detail/XMLUtils.h"

XERCES_CPP_NAMESPACE_USE

namespace gem {
    namespace onlinedb {

        bool AMC13Configuration::operator== (const AMC13Configuration &other) const
        {
            return getFEDId() == other.getFEDId()
                && isLocalTTCEnabled() == other.isLocalTTCEnabled()
                && getHostname() == other.getHostname();
        }

        void XMLSerializationData<AMC13Configuration>::readDOM(
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

                // Evaluate queries
                auto queryResultFEDId = std::unique_ptr<DOMXPathResult>();
                queryResultFEDId.reset(dom->evaluate(
                    "//DATA_SET/DATA/FED_ID"_xml,
                    dataSetsResult->getNodeValue(),
                    nullptr,
                    DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                    nullptr));

                auto queryResultTTC = std::unique_ptr<DOMXPathResult>();
                queryResultTTC.reset(dom->evaluate(
                    "//DATA_SET/DATA/ENABLE_LOCALTTC"_xml,
                    dataSetsResult->getNodeValue(),
                    nullptr,
                    DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                    nullptr));

                auto queryResultHostname = std::unique_ptr<DOMXPathResult>();
                queryResultHostname.reset(dom->evaluate(
                    "//DATA_SET/DATA/HOSTNAME"_xml,
                    dataSetsResult->getNodeValue(),
                    nullptr,
                    DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                    nullptr));

                // We assume that count is the same for all three queries, which
                // is the case if the XML is well-formed
                auto count = queryResultFEDId->getSnapshotLength();

                // Loop over DATA tags
                for (XMLSize_t i = 0; i < count; ++i) {
                    queryResultFEDId->snapshotItem(i);
                    queryResultTTC->snapshotItem(i);
                    queryResultHostname->snapshotItem(i);

                    auto FEDIdNode = queryResultFEDId->getNodeValue();
                    auto TTCNode = queryResultTTC->getNodeValue();
                    auto hostnameNode = queryResultHostname->getNodeValue();

                    auto config = AMC13Configuration();
                    config.setFEDId(
                        std::stoi(detail::transcode(FEDIdNode->getTextContent())));
                    config.setLocalTTCEnabled(
                        std::stoi(detail::transcode(TTCNode->getTextContent())));
                    config.setHostname(
                        detail::transcode(hostnameNode->getTextContent()));

                    dataSet.addData(config);
                }
            }
        }

        DOMDocumentPtr XMLSerializationData<AMC13Configuration>::makeDOM() const
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
                                    std::to_string((int) entry.isLocalTTCEnabled()));

                    auto hostnameElement = appendChildElement(data, "HOSTNAME");
                    appendChildText(hostnameElement, entry.getHostname());
                }
            }
            return dom;
        }

    } /* namespace onlinedb */
} /* namespace gem */
