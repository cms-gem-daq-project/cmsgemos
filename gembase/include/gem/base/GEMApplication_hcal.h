#ifndef gem_base_GEMApplication_H
#define gem_base_GEMApplication_H

#include <cstdlib>
#include "string.h"
#include "limits.h"
#include <string>
#include <string>
#include <deque>
#include <map>

#include "xdaq/Application.h"
#include "xdaq/WebApplication.h"

#include "log4cplus/logger.h"

#include "toolbox/BSem.h"
#include "toolbox/task/WorkLoop.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/mem/MemoryPoolFactory.h"

#include "xdata/Bag.h"
#include "xdata/Vector.h"
#include "xdata/InfoSpace.h"
#include "xdata/ActionListener.h"

#include "xdata/Float.h"
#include "xdata/String.h"
#include "xdata/Boolean.h"
#include "xdata/Integer.h"
#include "xdata/Integer64.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger.h"

#include "xoap/MessageReference.h"

#include "i2o/utils/AddressMap.h"

#include "xgi/Method.h"
#include "xdaq2rc/RcmsStateNotifier.h"

namespace cgicc {
  BOOLEAN_ELEMENT(section,"section");
}

namespace gem {
  namespace base {

    namespace cfg {
      class CfgDocument;
      class Record;
    }
    
    class GEMApplication : public xdaq::Application, public xdata::ActionListener
      {
      public:
        XDAQ_INSTANTIATOR();
	
        GEMApplication(xdaq::ApplicationStub *stub)
          throw (xdaq::exception::Exception);
	
        toolbox::fsm::FiniteStateMachine* getFSM(){ return &fsm_; }
	
        virtual void actionPerformed(xdata::Event&);
	
      protected:
        // FSM SOAP interface
        xoap::MessageReference fsmCallback(     xoap::MessageReference message)
          throw (xoap::exception::Exception);
        void fsmTransition(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
        /// Finite state machine callback (internal use)
        void fsmTransitionInternal(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
        /// Finite state machine callback (internal use)
        void fsmStateChanged (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);
        /// Finite state machine callbacks (internal use)
        bool steerInit(toolbox::task::WorkLoop* loop);
        bool steerWarm(toolbox::task::WorkLoop* loop);
        bool steerCold(toolbox::task::WorkLoop* loop);
        bool steerAsyncEnable(toolbox::task::WorkLoop* loop);
        bool steerAsyncDisable(toolbox::task::WorkLoop* loop);
        bool steerAsyncReset(toolbox::task::WorkLoop* loop);
        bool steerAsyncSuspend(toolbox::task::WorkLoop* loop);
        bool steerAsyncResume(toolbox::task::WorkLoop* loop);
        bool steerFix(toolbox::task::WorkLoop* loop);
	
        /// allow to update the progress of initialization as it goes forward (0->1)
        void updateProgress(float fractionalProgress);

        ///// Access the internal WebImageManager //may not need this, but if so, implement it for gem
        //hcal::toolbox::WebImageManager& getImageManager() { return m_imageManager; }
	
        /// Web callback for "default" action
        void xgiMenu(xgi::Input*, xgi::Output*) throw (xgi::exception::Exception);
        /// Web callback for looking at the application
        void xgiPeek(xgi::Input*, xgi::Output*) throw (xgi::exception::Exception);
        /// Web callback for reading the configuration
        void xgiCfg(xgi::Input*, xgi::Output*) throw (xgi::exception::Exception);
        /// Web callback for providing images and other data from the cache of "web data"
        void xgiData(xgi::Input*, xgi::Output*) throw (xgi::exception::Exception);
	
        /// allows value of the quantity to be updated
        virtual void actionPerformed(xdata::Event& e);

      protected:
        typedef enum CfgStyleType { cfg_NONE, cfg_XML, cfg_DOC } CfgStyle;
	
        GEMApplication(xdaq::ApplicationStub * s);
    
        /// set the configuration style of this application
        void setConfigurationStyle(CfgStyle style);
    
        /// get the configuration document
        gem::base::cfg::CfgDocument* getConfigurationDoc();
        /// release the configuration document
        void releaseConfigurationDoc(hcal::cfg::CfgDocument*);
    
        /// menu method  (default implemenation provides access to the peek and poke calls)
        virtual void webMenu(xgi::Input*, xgi::Output*) throw (gem::base::exception::Exception);
        /// peek method (default implementation shows the contents of the expert monitorable tree)
        virtual void webPeek(xgi::Input*, xgi::Output*) throw (gem::base::exception::Exception);
        /// poke method 
        virtual void webCfg(xgi::Input*, xgi::Output*) throw (gem::base::exception::Exception);
    
        /// actions to perform to go from Ready to Active
        virtual void enable() throw (gem::base::exception::Exception);
        /// actions to perform to go from Active to Paused
        virtual void pause() throw (gem::base::exception::Exception);
        /// actions to perform to go from Paused to Active
        virtual void resume() throw (gem::base::exception::Exception);
        /// actions to perform to go from Paused or Active to Ready
        virtual void disable() throw (gem::base::exception::Exception);  
        /// actions to perform in the state COLD.  Automatic transition to INIT after successful completion [Asynchronous]
        virtual void coldInit() throw (gem::base::exception::Exception);
        /// actions to perform in the state INIT.  Automatic transition to READY after successful completion [Asynchronous]
        virtual void init() throw (gem::base::exception::Exception);
        /// actions to perform in the state WARM INIT.  Automatic transition to READY after successful completion [Asynchronous]
        virtual void warmInit() throw (gem::base::exception::Exception);
        /// actions to perform to go from Uninit to PREINIT 
        virtual void preInit() throw (gem::base::exception::Exception);
        /// actions to perform to go from Ready or Failed to Uninit
        virtual void reset() throw (gem::base::exception::Exception);

        /// actions to perform to go from Ready to Active
        virtual void enableAsync() throw (gem::base::exception::Exception);
        /// actions to perform to go from Active to Paused
        virtual void pauseAsync() throw (gem::base::exception::Exception);
        /// actions to perform to go from Paused to Active
        virtual void resumeAsync() throw (gem::base::exception::Exception);
        /// actions to perform to go from Paused or Active to Ready
        virtual void disableAsync() throw (gem::base::exception::Exception);  
        /// actions to perform to go from Ready or Failed to Uninit
        virtual void resetAsync() throw (gem::base::exception::Exception);
        /// actions to perform during fixing (async)
        virtual void fix() throw (gem::base::exception::Exception);
    
        void fsmCommand(const std::string& command) throw (gem::base::exception::Exception);
    
        /// access the configuration database (sets a lock, must release)
        ConfigurationDatabase* getCfgDatabase(const char* accessor=0) throw (gem::base::exception::ConfigurationDatabaseException);
        /// release the lock on the configuration database
        void releaseCfgDatabase(ConfigurationDatabase** theDB);
        /// access the conditions database (sets a lock, must release)
        ConditionsDatabase* getCondDatabase(const char* accessor=0) throw (gem::base::exception::ConditionsDatabaseException);
        /// release the lock on the conditions database
        void releaseCondDatabase(ConditionsDatabase** theDB);
        /// internal request to change state
        void changeState(toolbox::fsm::State newState);
        /// send event dependent or run dependent information to the relevant registry
        void postInfo(bool eventDependent, const std::map<std::string, std::string>& strValues, const std::map<std::string, double>& numValues);
    
        /// get name for state
        std::string getStateName(toolbox::fsm::State st);

        /// access the expert-view monitoring container
        gem::base::monitor::Bag* getExpertView();
        /// access the automatic monitoring container
        gem::base::monitor::Bag* getAutoView();

        /// access the CGI renderer to change settings
        inline gem::base::toolbox::CGIRenderer& getRenderer() { return m_renderer; }
    
        /// create the URL to access the WebData of the given name
        std::string getLinkForCGIData(const std::string& name);
        /// create the URL to access the Peek item of the given name
        std::string getLinkForPeek(const std::string& itemName);
    
        /// make quantity available to monitoring system
        void exportMonitorable(gem::base::monitor::Monitorable* m);

        /// set the workloop name (rarely needed)
        void setWorkloopName(const std::string& name) { m_workLoopName=name; }

        // post an alarm
        enum AlarmSeverity { al_UNKNOWN=0, al_WARNING=1, al_MINOR=2, al_ERROR=3, al_FATAL=4 };

        /// Post an alarm with the given severity and name using an explicit exception object
        void postAlarm(AlarmSeverity severity, const std::string& name, xcept::Exception& exception);

        /// Revoke the given alarm
        void revokeAlarm(const std::string& name);

        /// Check for the existance of a given alarm
        bool hasAlarm(const std::string& name);

      protected:
        std::string getRawCfgDocument() const { return m_cfgDocumentText.value_; }
        void replaceRawCfgDocument(const std::string& content);
        int getRunNumber() const { return m_runNumber.value_; }
        xdata::InfoSpace* getMonitorInfospace() { return m_monitorInfospace; }
        std::string condAccessor() const { return m_condDatabaseAccessor.value_; }
        std::string cfgAccessor() const { return m_databaseAccessor.value_; }
        bool m_saveRecordOnReInit;

      private:
        xdata::String m_databaseAccessor, m_condDatabaseAccessor;
        void startAsync(const std::string& command) throw (gem::base::exception::Exception);
        void startInit() throw (gem::base::exception::Exception);
        void buildCfgWebpage();
        toolbox::fsm::FiniteStateMachine gemAppFSM;
        xdata::Float  m_progress;
        xdata::String m_stateTransitionMessage, m_stateReporter;
        xdata::String m_cfgStyle, m_cfgDocumentText, m_cfgDocumentFilename;
        CfgStyle m_cfgStyleCode;
        xdata::Integer m_runNumber;
        toolbox::BSem m_databaseLock, m_condLock ;
        ConfigurationDatabase* p_database;
        ConditionsDatabase*    p_conddatabase;
        gem::base::monitor::Bag *m_expertView, *m_autoView;
        gem::base::toolbox::CGIRenderer m_renderer;
        gem::base::toolbox::WebImageManager m_imageManager;
        gem::base::cfg::Record* m_cfgRecord;
        std::string m_cfgWebpage; // pre-calculated image to save time in the future (and avoid multiple reload problem)
        gem::base::cfg::CfgDocument* m_cfgDocument;

        std::string m_endOfFixCommand; // needed to return to proper state after fix
        std::string m_workLoopName;
        //added for Monitoring
        xdata::InfoSpace* m_monitorInfospace; 
        std::map<std::string,gem::base::monitor::Monitorable*> m_monitorables;
    
      };
    
  } // namespace gem::base
} // namespace gem

#endif
