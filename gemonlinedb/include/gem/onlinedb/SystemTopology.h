#ifndef GEM_ONLINEDB_SYSTEMTOPOLOGY_H
#define GEM_ONLINEDB_SYSTEMTOPOLOGY_H

#include <array>
#include <vector>

#include "gem/onlinedb/AMC13Configuration.h"
#include "gem/onlinedb/AMCConfiguration.h"
#include "gem/onlinedb/GBTXConfiguration.h"
#include "gem/onlinedb/OHv3Configuration.h"
#include "gem/onlinedb/VFAT3ChipConfiguration.h"

#include "gem/onlinedb/ConfigurationTraits.h"
#include "gem/onlinedb/PartReference.h"

#include "gem/onlinedb/detail/XMLUtils.h"

namespace gem {
    namespace onlinedb {

        /**
         * @brief Describes the topology of a system.
         *
         * The description is built as a tree of nodes containing part
         * references for the various components. The root nodes are the AMC 13,
         * under which AMCs are found, then OHs and VFATs.
         *
         * The tree can be read from an XML document that adheres to the
         * @c system-topology schema.
         */
        class SystemTopology
        {
        public:
            /**
             * @brief Represents a VFAT 3.
             */
            class VFAT3Node
            {
                friend class SystemTopology;

            public:
                /**
                 * @brief The part reference associated to the VFAT.
                 * @note The @c PartType for @ref VFAT3ChannelConfiguration
                 *       isn't used.
                 */
                ConfigurationTraits<VFAT3ChipConfiguration>::PartType reference;

            private:
                /// @brief Populates a @c VFAT3Node from XML data.
                void populate(const xercesc::DOMElement *el);
            };

            /**
             * @brief Represents a GBTX.
             */
            class GBTXNode
            {
                friend class SystemTopology;

            public:
                /**
                 * @brief The part reference associated to the GBTX.
                 */
                ConfigurationTraits<GBTXConfiguration>::PartType reference;

            private:
                /// @brief Populates a @c GBTXNode from XML data.
                void populate(const xercesc::DOMElement *el);
            };

            /**
             * @brief Represents an OHv3.
             */
            class OHv3Node
            {
                static const constexpr std::size_t GBTX_COUNT = 3;
                static const constexpr std::size_t VFAT_COUNT = 24;

                friend class SystemTopology;

            public:
                /// @brief The part reference associated to the OH.
                ConfigurationTraits<OHv3Configuration>::PartType reference;

                /// @brief The child VFATs (entries can be null).
                std::array<std::unique_ptr<GBTXNode>, GBTX_COUNT> gbtx;

                /// @brief The child VFATs (entries can be null).
                std::array<std::unique_ptr<VFAT3Node>, VFAT_COUNT> vfat;

            private:
                /// @brief Populates a @c OHv3Node from XML data.
                void populate(const xercesc::DOMElement *el);
            };

            /**
             * @brief Represents an AMC.
             */
            class AMCNode
            {
                static const constexpr std::size_t OH_COUNT = 12;

                friend class SystemTopology;

            public:
                /// @brief The part reference associated to the AMC
                ConfigurationTraits<AMCConfiguration>::PartType reference;

                /// @brief The connected optohybrids (entries can be null).
                std::array<std::unique_ptr<OHv3Node>, OH_COUNT> oh;

            private:
                /// @brief Populates an @c AMCNode from XML data.
                void populate(const xercesc::DOMElement *el);
            };

            /**
             * @brief Represents an AMC13.
             */
            class AMC13Node
            {
                friend class SystemTopology;

            public:
                /// @brief The part reference associated to the AMC13
                ConfigurationTraits<AMC13Configuration>::PartType reference;

                /// @brief The connected AMCs (entries cannot be null).
                std::vector<AMCNode> amc;

            private:
                /// @brief Populates an @c AMC13Node from XML data.
                void populate(const xercesc::DOMElement *el);
            };

            std::vector<AMC13Node> m_roots;

        public:
            /// @brief Constructor.
            explicit SystemTopology() = default;

            /// @brief Destructor.
            virtual ~SystemTopology() = default;

            /**
             * @brief Populates the system tree be reading the contents of
             *        @c document.
             */
            void populate(const DOMDocumentPtr &document);

            /// @brief Returns the root nodes of the system trees.
            const std::vector<AMC13Node> &roots() const { return m_roots; };
        };

    } // namespace onlinedb
} // namespace gem


#endif // GEM_ONLINEDB_SYSTEMTOPOLOGY_H
