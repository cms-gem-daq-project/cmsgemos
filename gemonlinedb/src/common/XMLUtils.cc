#include "gem/onlinedb/detail/XMLUtils.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "gem/onlinedb/exception/Exception.h"

namespace gem {
    namespace onlinedb {
        namespace detail {

            std::string transcode(const XMLCh * const xercesStr)
            {
                char *transcoded = xercesc::XMLString::transcode(xercesStr);
                std::string result(transcoded);
                xercesc::XMLString::release(&transcoded);
                return result;
            }

            std::string transcodeAndRelease(XMLCh *xercesStr)
            {
                std::string transcoded = transcode(xercesStr);
                xercesc::XMLString::release(&xercesStr);
                return transcoded;
            }

            XercesGuard::XercesGuard()
            {
                try {
                    // Initialize Xerces
                    xercesc::XMLPlatformUtils::Initialize();
                } catch (const xercesc::XMLException &e) {
                    XCEPT_RAISE(exception::SoftwareProblem,
                                std::string("Xerces failed to initialize: ") +
                                    detail::transcode(e.getMessage()));
                }
            }

            XercesGuard::~XercesGuard()
            {
                xercesc::XMLPlatformUtils::Terminate();
            }

        } /* namespace detail */
    } /* namespace onlinedb */
} /* namespace gem */
