#ifndef gem_supervisor_GEMGLIBSupervisorWeb_h
#define gem_supervisor_GEMGLIBSupervisorWeb_h

#include "xdaq/Application.h"
#include "xdaq/WebApplication.h"

#include "xdata/Float.h"
#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/task/WorkLoop.h"
#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/Event.h"
#include "toolbox/BSem.h"

#include "xdaq/NamespaceURI.h"
#include "xoap/Method.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPSerializer.h"
#include "xoap/domutils.h"

#include "xoap/DOMParser.h"
#include "xoap/DOMParserFactory.h"
#include "xoap/domutils.h"
#include "xdata/soap/Serializer.h"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

#include <string>

#include "xdata/UnsignedInteger64.h"
#include "xdata/UnsignedInteger.h"
#include "xdata/UnsignedShort.h"
#include "xdata/Integer.h"

typedef uhal::exception::exception uhalException;

namespace gem {
    namespace hw {
        namespace vfat {
            class HwVFAT2;
        }   
    }
    namespace supervisor {

        class GEMGLIBSupervisorWeb: public xdaq::WebApplication
        {
            public:
                XDAQ_INSTANTIATOR();

                GEMGLIBSupervisorWeb(xdaq::ApplicationStub * s)
                    throw (xdaq::exception::Exception);

                // SOAP interface
                xoap::MessageReference onConfigure(xoap::MessageReference message)
                    throw (xoap::exception::Exception);
                xoap::MessageReference onStart(xoap::MessageReference message)
                    throw (xoap::exception::Exception);
                xoap::MessageReference onStop(xoap::MessageReference message)
                    throw (xoap::exception::Exception);
                xoap::MessageReference onHalt(xoap::MessageReference message)
                    throw (xoap::exception::Exception);

                // HyperDAQ interface
                void webDefault(xgi::Input *in, xgi::Output *out)
                    throw (xgi::exception::Exception);
                void webConfigure(xgi::Input *in, xgi::Output *out)
                    throw (xgi::exception::Exception);
                void webStart(xgi::Input *in, xgi::Output *out)
                    throw (xgi::exception::Exception);
                void webStop(xgi::Input *in, xgi::Output *out)
                    throw (xgi::exception::Exception);
                void webHalt(xgi::Input *in, xgi::Output *out)
                    throw (xgi::exception::Exception);
                void webTrigger(xgi::Input *in, xgi::Output *out)
                    throw (xgi::exception::Exception);
                void webRedirect(xgi::Input *in, xgi::Output *out)
                    throw (xgi::exception::Exception);

                // work loop call-back functions
                bool configureAction(toolbox::task::WorkLoop *wl);
                bool stopAction(toolbox::task::WorkLoop *wl);
                bool haltAction(toolbox::task::WorkLoop *wl);
                bool startAction(toolbox::task::WorkLoop *wl);
                bool runAction(toolbox::task::WorkLoop *wl);
                bool readAction(toolbox::task::WorkLoop *wl);

                // State transitions
                void configureAction(toolbox::Event::Reference e)
                    throw (toolbox::fsm::exception::Exception);
                void startAction(toolbox::Event::Reference e)
                    throw (toolbox::fsm::exception::Exception);
                void stopAction(toolbox::Event::Reference e)
                    throw (toolbox::fsm::exception::Exception);
                void haltAction(toolbox::Event::Reference e)
                    throw (toolbox::fsm::exception::Exception);
                void noAction(toolbox::Event::Reference e)
                    throw (toolbox::fsm::exception::Exception);

                class ConfigParams 
                {   
                    public:
                        void registerFields(xdata::Bag<ConfigParams> *bag);

                        xdata::UnsignedInteger latency;

                        xdata::String        outFileName;

                        xdata::String        deviceName;
                        xdata::String        deviceIP;
                        xdata::Integer       deviceNum;
                        xdata::UnsignedShort triggerSource;
                        xdata::UnsignedShort deviceChipID;
                        xdata::UnsignedShort deviceVT1;
                        xdata::UnsignedShort deviceVT2;
                };

            private:
                toolbox::task::WorkLoop *wl_;

                toolbox::BSem wl_semaphore_;
                toolbox::BSem hw_semaphore_;

                toolbox::task::ActionSignature *configure_signature_;
                toolbox::task::ActionSignature *stop_signature_;
                toolbox::task::ActionSignature *halt_signature_;
                toolbox::task::ActionSignature *start_signature_;
                toolbox::task::ActionSignature *run_signature_;
                toolbox::task::ActionSignature *read_signature_;

                toolbox::fsm::FiniteStateMachine fsm_;

                xdata::Bag<ConfigParams> confParams_;

                FILE* outputFile;

                uint64_t latency_;

                bool is_working_, is_initialized_, is_configured_, is_running_;

                gem::hw::vfat::HwVFAT2* vfatDevice_;

                int counter_;

                void fireEvent(std::string name);
                void stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
                    throw (toolbox::fsm::exception::Exception);
                void transitionFailed(toolbox::Event::Reference event)
                    throw (toolbox::fsm::exception::Exception);
        };
    }
}
#endif
