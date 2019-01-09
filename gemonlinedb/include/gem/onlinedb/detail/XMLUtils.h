#ifndef GEM_ONLINEDB_DETAIL_XMLUTILS_H
#define GEM_ONLINEDB_DETAIL_XMLUTILS_H

#include <string>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>

/**
 * @file
 * @brief Defines utilities to make Xerces slightly less verbose.
 */

namespace gem {
    namespace onlinedb {
        namespace detail {
            /**
             * @brief Wraps a Xerces string and xerces::XMLString::transcode
             *        operation.
             *
             * The associated memory is freed upon destruction of the object.
             */
            class XercesString final
            {
                XMLCh *m_transcoded;

            public:
                /**
                 * @brief Constructor.
                 */
                XercesString(const char * const literal)
                {
                    m_transcoded = xercesc::XMLString::transcode(literal);
                }

                /**
                 * @brief Constructor.
                 */
                XercesString(const std::string &string)
                {
                    m_transcoded = xercesc::XMLString::transcode(string.c_str());
                }

                /**
                 * @brief Copies are not allowed.
                 */
                XercesString(const XercesString &) = delete;

                /**
                 * @brief Move constructor.
                 */
                XercesString(const XercesString &&other)
                {
                    m_transcoded = other.m_transcoded;
                }

                /**
                 * @brief Destructor.
                 */
                ~XercesString()
                {
                    xercesc::XMLString::release(&m_transcoded);
                }

                /**
                 * @brief Allows one to pass objects of this type to Xerces
                 *        functions.
                 */
                operator const XMLCh * const () const
                {
                    return m_transcoded;
                }

            };

            /**
             * @brief Transcodes a Xerces string to an std::string in local
             *        encoding.
             */
            std::string transcode(const XMLCh * const xercesStr);

            /**
             * @brief Transcodes a Xerces string to an std::string in local
             *        encoding, and releases the input memory.
             */
            std::string transcodeAndRelease(XMLCh *xercesStr);

            /**
             * @brief Defines the @c ""_xml literal to declare Xerces strings.
             */
            namespace literals {
                /**
                 * @brief Creates Xerces strings from string literals.
                 */
                inline XercesString operator"" _xml (const char *string,
                                                     std::size_t)
                {
                    return XercesString(string);
                }
            } /* namespace literals */

            /**
             * @brief Takes care of initializing and finalizing the Xerces
             *        library.
             */
            class XercesGuard
            {
            public:
                /**
                 * @brief Initializes Xerces.
                 */
                XercesGuard();

                /**
                 * @brief Finalizes Xerces.
                 */
                ~XercesGuard();
            };

            /**
             * @brief Appends a child element to @c parent with tag name
             *        @c tagName.
             *
             * The new element is created without any attribute or child.
             * @returns The new element.
             */
            xercesc::DOMElement *appendChildElement(
                xercesc::DOMElement *parent, const std::string &tagName);

            /**
             * @brief Appends a child text element to @c parent, with given
             *        @c contents.
             * @returns The new text node.
             */
            xercesc::DOMText *appendChildText(
                xercesc::DOMElement *parent, const std::string &content);

        } /* namespace detail */
    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_DETAIL_XMLUTILS_H
