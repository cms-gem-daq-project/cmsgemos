#ifndef GEM_ONLINEDB_DETAIL_XMLUTILS_H
#define GEM_ONLINEDB_DETAIL_XMLUTILS_H

#include <memory>
#include <mutex>
#include <string>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/XMLString.hpp>

#include "gem/onlinedb/exception/Exception.h"

/**
 * @file
 * @brief Defines utilities to make Xerces slightly less verbose.
 */

namespace gem {
    namespace onlinedb {
        /**
         * @brief Convenience alias.
         */
        using DOMDocumentPtr = std::unique_ptr<xercesc::DOMDocument>;

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
             *
             * @note Exceptions thrown by Xerces rely on the library being
             *       loaded. Code guarded by this class needs to catch
             *       exceptions.
             */
            class XercesGuard
            {
                static std::recursive_mutex s_mutex;

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
             * @brief Xerces error handler that throws on every error or warning.
             * @throws exception::ParseError
             */
            class XercesAlwaysThrowErrorHandler : public xercesc::ErrorHandler
            {
            public:
                void warning(const xercesc::SAXParseException &e) override
                {
                    XCEPT_RAISE(exception::ParseError,
                        transcode(e.getPublicId()) + ": " +
                        std::to_string(e.getLineNumber()) + ":" +
                        std::to_string(e.getColumnNumber()) + ": " +
                        transcode(e.getMessage()));
                };

                void error(const xercesc::SAXParseException &e) override
                {
                    XCEPT_RAISE(exception::ParseError,
                        transcode(e.getPublicId()) + ": " +
                        std::to_string(e.getLineNumber()) + ":" +
                        std::to_string(e.getColumnNumber()) + ": " +
                        transcode(e.getMessage()));
                };

                void fatalError(const xercesc::SAXParseException &e) override
                {
                    XCEPT_RAISE(exception::ParseError,
                        transcode(e.getPublicId()) + ": " +
                        std::to_string(e.getLineNumber()) + ":" +
                        std::to_string(e.getColumnNumber()) + ": " +
                        transcode(e.getMessage()));
                };

                void resetErrors() override {};
            };

            /**
             * @brief Turns Xerces exceptions thrown by a function or functor
             *        object into @ref ParseError.
             *
             * This function creates a @ref XercesGuard.
             */
            template<class F, class... Args>
            auto xercesExceptionsToXcept(F f, Args... args) ->
                decltype(f(std::forward<Args>(args)...))
            {
                XERCES_CPP_NAMESPACE_USE
                using exception::ParseError;

                XercesGuard guard;
                try {
                    return f(std::forward<Args>(args)...);
                } catch(DOMException &e) {
                    XCEPT_RAISE(ParseError, transcode(e.getMessage()));
                } catch (XMLException &e) {
                    XCEPT_RAISE(ParseError, transcode(e.getMessage()));
                } catch (SAXParseException &e) {
                    XCEPT_RAISE(ParseError,
                        transcode(e.getPublicId()) + ": " +
                        std::to_string(e.getLineNumber()) + ":" +
                        std::to_string(e.getColumnNumber()) + ": " +
                        transcode(e.getMessage()));
                } catch (SAXException &e) {
                    XCEPT_RAISE(ParseError, transcode(e.getMessage()));
                }
            }

            /**
             * @brief Turns Xerces exceptions thrown by a function or functor
             *        object into @ref std::runtime_error.
             *
             * This function creates a @ref XercesGuard.
             */
            template<class F, class... Args>
            auto xercesExceptionsToStd(F f, Args... args) ->
                decltype(f(std::forward<Args>(args)...))
            {
                try {
                    return xercesExceptionsToXcept(f, std::forward<Args>(args)...);
                } catch(xcept::Exception &e) {
                    // Use the opportunity to convert all Xcept exceptions
                    throw std::runtime_error(e.message());
                }
            }

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


            /**
             * @brief Executes an XSD query on the given document and retrieves
             *        the corresponding DOMNode.
             */
            xercesc::DOMNode *xsdGet(const DOMDocumentPtr &document,
                                     const char *query);

            /**
             * @brief Executes an XSD query on the given document and retrieves
             *        the text content of the corresponding DOMNode.
             */
            std::string xsdGetTextContent(const DOMDocumentPtr &document,
                                          const char *query);

            /**
             * @brief Finds the first direct child of @c root with the given
             *        @c tagName
             */
            xercesc::DOMElement *findChildElement(
                const xercesc::DOMElement *root, const std::string &tagName);

        } /* namespace detail */
    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_DETAIL_XMLUTILS_H
