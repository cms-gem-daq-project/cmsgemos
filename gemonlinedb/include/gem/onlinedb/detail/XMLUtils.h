#ifndef _XMLUtils_h_
#define _XMLUtils_h_

#include <string>

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
                XMLCh *transcoded;

            public:
                /**
                 * @brief Constructor.
                 */
                XercesString(const char * const literal)
                {
                    transcoded = xercesc::XMLString::transcode(literal);
                }

                /**
                 * @brief Constructor.
                 */
                XercesString(const std::string &string)
                {
                    transcoded = xercesc::XMLString::transcode(string.c_str());
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
                    transcoded = other.transcoded;
                }

                /**
                 * @brief Destructor.
                 */
                ~XercesString()
                {
                    xercesc::XMLString::release(&transcoded);
                }

                /**
                 * @brief Allows one to pass objects of this type to Xerces
                 *        functions.
                 */
                operator const XMLCh * const () const
                {
                    return transcoded;
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

        } /* namespace detail */
    } /* namespace onlinedb */
} /* namespace gem */

#endif // _XMLUtils_h_
