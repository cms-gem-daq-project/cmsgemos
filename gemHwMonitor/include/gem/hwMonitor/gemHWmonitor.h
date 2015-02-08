#ifndef gem_hwMon_gemHWmonitor_h
#define gem_hwMon_gemHWmonitor_h

#include <string>

#include "xdaq/Application.h"

namespace gem {
    namespace hwMon {

        class gemHWmonitor: public xdaq::Application//, public xdata::ActionListener
        {
            public:
                XDAQ_INSTANTIATOR();
                gemHWmonitor(xdaq::ApplicationStub * s)
                    throw (xdaq::exception::Exception);

                ~gemHWmonitor(){}
                /**
                 *   Select one of available crates
                 */
                void setCurrentCrate(std::string *crateID){currentCrateID_ = crateID}
                    throw (xgi::exception::Exception);
                /**
                 *   Get selected crate
                 */
                std::string getCurrentCrate(){return currentCrateID_;}
                    throw (xgi::exception::Exception);
                /**
                 *   Select XML configuration file
                 */
                void setXMLconfigFile (std::string inputXMLfilename){}
                    throw (xgi::exception::Exception);
                /**
                 *   Select database configuration file
                 */
                void setDBSconfigFile (std::string inputDBSfilename){}
                    throw (xgi::exception::Exception);
                /**
                 *   Board status
                 */
                unsigned int getBoardStatus (){return boardStatus_;}
                    throw (xgi::exception::Exception);
                /**
                 *   Access to board utils
                 */
                virtual void boardUtils ()
                    throw (xgi::exception::Exception);

            protected:
            private:
                std::string currentCrateID_;
                unsigned int boardStatus_;
                std::string boardID_;
        }; // end namespace hwMon
    }
} // end namespace gem
#endif
