#ifndef GEM_ONLINEDB_SYSTEMTOPOLOGY_H
#define GEM_ONLINEDB_SYSTEMTOPOLOGY_H

#include <array>
#include <vector>

#include "gem/onlinedb/AMC13Configuration.h"
#include "gem/onlinedb/AMCConfiguration.h"
#include "gem/onlinedb/OHv3Configuration.h"
#include "gem/onlinedb/VFAT3ChipConfiguration.h"

#include "gem/onlinedb/ConfigurationTraits.h"
#include "gem/onlinedb/PartReference.h"

namespace gem {
    namespace onlinedb {

        class SystemTopology
        {
        private:
            struct VFAT3Node
            {
                // We don't use VFAT3ChannelConfiguration's PartType
                ConfigurationTraits<VFAT3ChipConfiguration>::PartType reference;

                void populate(const xercesc::DOMElement *el);
            };

            struct OHv3Node
            {
                static const constexpr std::size_t VFAT_COUNT = 24;

                ConfigurationTraits<OHv3Configuration>::PartType reference;
                std::array<std::unique_ptr<VFAT3Node>, VFAT_COUNT> vfat;

                void populate(const xercesc::DOMElement *el);
            };

            struct AMCNode
            {
                static const constexpr std::size_t OH_COUNT = 12;

                ConfigurationTraits<AMCConfiguration>::PartType reference;
                std::array<std::unique_ptr<OHv3Node>, OH_COUNT> oh;

                void populate(const xercesc::DOMElement *el);
            };

            struct AMC13Node
            {
                ConfigurationTraits<AMC13Configuration>::PartType reference;
                std::vector<AMCNode> amc;

                void populate(const xercesc::DOMElement *el);
            };

            AMC13Node m_root;

        public:
            explicit SystemTopology() = default;
            virtual ~SystemTopology() = default;

            void populate(const DOMDocumentPtr &document);
        };

    } // namespace onlinedb
} // namespace gem


#endif // GEM_ONLINEDB_SYSTEMTOPOLOGY_H
