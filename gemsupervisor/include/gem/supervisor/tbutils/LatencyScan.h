#ifndef gem_supervisor_tbutils_LatencyScan_h
#define gem_supervisor_tbutils_LatencyScan_h

#include <map>
#include <string>

#include "log4cplus/logger.h"

#include "toolbox/Event.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/lang/Class.h"
#include "xcept/Exception.h"
#include "xdaq2rc/RcmsStateNotifier.h"
#include "xoap/MessageReference.h"
#include "xcept/Exception.h"

namespace toolbox {
  namespace fsm {
    class AsynchronousFiniteStateMachine;
  }
}

namespace gem {
  namespace supervisor {
    namespace tbutils {

      class LatencyScan : virtual public toolbox::lang::Class
	{
	  
	public:
	  LatencyScan();
	  ~LatencyScan();

	  xoap::MessageReference changeState(xoap::MessageReference msg);
	  void stateChanged(toolbox::fsm::FiniteStateMachine& fsm)

	  configure();

	private:
	  toolbox::fsm::AsynchronousFiniteStateMachine* fsmP_;

	  toolbox::task::ActionSignature* initSig_;
	  toolbox::task::ActionSignature* confSig_;
	  toolbox::task::ActionSignature* startSig_;
	  toolbox::task::ActionSignature* stopSig_;

	  int nTriggers_, nSteps_, minThresh_, maxThresh_;

	protected:
	  
	  
	};

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
