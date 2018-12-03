#ifndef _XMLWriter_h_
#define _XMLWriter_h_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <xercesc/dom/DOMElement.hpp>

#include "gem/onlinedb/ConfigurationTraits.h"
#include "gem/onlinedb/DataSet.h"
#include "gem/onlinedb/PartReference.h"
#include "gem/onlinedb/Run.h"
#include "gem/onlinedb/detail/RegisterData.h"

namespace gem {
    namespace onlinedb {

        /**
         * @brief Convenience alias.
         */
        using DOMDocumentPtr = std::unique_ptr<xercesc::DOMDocument>;

        /**
         * @brief Class used to create XML files.
         * @tparam ConfigurationTypeT The configuration class that will be serialized.
         */
        template<class ConfigurationTypeT>
        class XMLBuilder
        {
        public:
            /**
             * @brief Convenience typedef.
             */
            using ConfigurationType = ConfigurationTypeT;

        private:
            Run run;
            std::vector<DataSet<ConfigurationType>> dataSets;

        public:
            /**
             * @brief Sets the run for which to generate XML data.
             */
            void setRun(const Run &run) { this->run = run; }

            /**
             * @brief Adds a dataset to be serialized
             */
            void addDataSet(const DataSet<ConfigurationType> &dataSet)
            {
                dataSets.push_back(dataSet);
            }

            /**
             * @brief Generates a DOM document representing data previously set
             *        using @ref setRun and @ref addDataSet.
             */
            DOMDocumentPtr makeDOM() const;
        };

        // Utility functions for makeDOM
        namespace detail {
            /**
             * @brief Creates a DOM document to record data into.
             * @param extTableName The name of the database table to record data in.
             * @param comment A free-form comment.
             * @param run The run the data will be associated to.
             * @throw exception::SoftwareProblem If some Xerces object cannot be created.
             */
            DOMDocumentPtr makeDOM(const std::string &extTableName,
                                   const std::string &comment,
                                   const Run &run);

            /**
             * @brief Creates a DATA_SET node inside a DOM document.
             * @param document A DOM document created by @ref makeDOM.
             * @param comment A free-form comment.
             * @param version The version of the data set.
             */
            xercesc::DOMElement *createDataSetElement(DOMDocumentPtr &document,
                                                      const std::string &comment,
                                                      const std::string &version);

            /**
             * @brief Appends a PART element to the given parent.
             * @param kindOfPart The kind of part configured.
             * @param partRef A way of identifying the part.
             */
            void createPartElement(xercesc::DOMElement *parent,
                                   const std::string &kindOfPart,
                                   const PartReferenceBarcode &partRef);

            /**
             * @brief Appends a PART element to the given parent.
             * @param kindOfPart The kind of part configured.
             * @param partRef A way of identifying the part.
             */
            void createPartElement(xercesc::DOMElement *parent,
                                   const std::string &kindOfPart,
                                   const PartReferenceSN &partRef);

            /**
             * @brief Appends a DATA element to the given parent.
             * @param content The contents of the DATA element.
             */
            void createDataElement(xercesc::DOMElement *parent,
                                   const RegisterData &content);
        } /* namespace detail */

        template<class ConfigurationType>
        DOMDocumentPtr XMLBuilder<ConfigurationType>::makeDOM() const
        {
            using Info = ConfigurationTraits<ConfigurationType>;

            // Create the general DOM structure
            auto dom = detail::makeDOM(Info::extTableName(), Info::typeName(), run);

            for (const auto &dataSet : dataSets) {
                // Add a DATA_SET node
                auto dataSetNode = detail::createDataSetElement(dom,
                                                                dataSet.comment,
                                                                dataSet.version);
                // Create the PART element
                detail::createPartElement(dataSetNode, Info::kindOfPart(), dataSet.part);
                // Add DATA elements
                for (const auto &entry : dataSet.data) {
                    detail::createDataElement(dataSetNode,
                                              entry.getRegisterData());
                }
            }
            return dom;
        }

    } /* namespace onlinedb */
} /* namespace gem */

#endif // _XMLWriter_h_
