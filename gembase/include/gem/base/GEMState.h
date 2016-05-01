#ifndef GEM_BASE_GEMSTATES_H
#define GEM_BASE_GEMSTATES_H

// Basic implementation copied from HCAL code

namespace gem {
  namespace base {
    // namespace states {
      static const toolbox::fsm::State STATE_UNINIT      = 'U';  ///< Uninitialized state (power on, reset, and recovery state)
      static const toolbox::fsm::State STATE_COLD        = 'B';  ///< Cold initialization state (firmware reload)
      static const toolbox::fsm::State STATE_INITIAL     = 'I';  ///< Initial state
      static const toolbox::fsm::State STATE_HALTED      = 'H';  ///< Halted state
      static const toolbox::fsm::State STATE_CONFIGURED  = 'C';  ///< Configured state
      static const toolbox::fsm::State STATE_RUNNING     = 'E';  ///< Running (enabled, active) state
      static const toolbox::fsm::State STATE_PAUSED      = 'P';  ///< Paused state
      static const toolbox::fsm::State STATE_FAILED      = 'F';  ///< Failed state

      // transitional states, TCDS way seems more elegant than HCAL, but both use a similar idea
      static const toolbox::fsm::State STATE_INITIALIZING = 'i';  ///< Initializing transitional state
      static const toolbox::fsm::State STATE_CONFIGURING  = 'c';  ///< Configuring transitional state
      static const toolbox::fsm::State STATE_HALTING      = 'h';  ///< Halting transitional state
      static const toolbox::fsm::State STATE_PAUSING      = 'p';  ///< Pausing transitional state
      static const toolbox::fsm::State STATE_STOPPING     = 's';  ///< Stopping transitional state
      static const toolbox::fsm::State STATE_STARTING     = 'e';  ///< Starting transitional state
      static const toolbox::fsm::State STATE_RESUMING     = 'r';  ///< Resuming transitional state
      static const toolbox::fsm::State STATE_RESETTING    = 't';  ///< Resetting transitional state
      static const toolbox::fsm::State STATE_FIXING       = 'X';  ///< Fixing transitional state

      static const toolbox::fsm::State STATE_NULL         = 0;    ///< Null state
      // }  // namespace gem::base::states
  }  // namespace gem::base
}  // namespace gem

#endif  // GEM_BASE_GEMSTATES_H
