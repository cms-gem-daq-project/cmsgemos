package rcms.fm.gem.gemLevelOne;

import rcms.fm.fw.parameter.CommandParameter;
import rcms.fm.fw.parameter.ParameterException;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.parameter.type.IntegerT;
import rcms.fm.fw.parameter.type.StringT;
import rcms.fm.fw.user.UserStateMachineDefinition;
import rcms.statemachine.definition.State;
import rcms.statemachine.definition.StateMachineDefinitionException;

/**
 * This class defines the Finite State Machine for the GEM level 1 Function Manager.
 * 
 * The actual definition of the State Machine must be put in the "init" method.
 * 
 * @originalauthor Andrea Petrucci, Alexander Oh, Michele Gulmini
 * @author Jared Sturdy
 */
public class GEMStateMachineDefinition extends UserStateMachineDefinition {
    
    public GEMStateMachineDefinition() throws StateMachineDefinitionException {
	//
	// Defines the States for this Finite State Machine.
	//
	
	// steady states
	addState(GEMStates.INITIAL     );
	addState(GEMStates.ENABLED     );
	addState(GEMStates.HALTED      );
	addState(GEMStates.CONFIGURED  );
	addState(GEMStates.RUNNING     );
	addState(GEMStates.PAUSED      );
	addState(GEMStates.ERROR       );

	addState(GEMStates.TTSTEST_MODE);
		
	// transitional states
	addState(GEMStates.INITIALIZING);
	addState(GEMStates.ENABLING    );
	addState(GEMStates.CONFIGURING );
	addState(GEMStates.HALTING     );
	addState(GEMStates.PAUSING     );
	addState(GEMStates.RESUMING    );
	addState(GEMStates.STARTING    );
	addState(GEMStates.STOPPING    );
	addState(GEMStates.RECOVERING  );
	addState(GEMStates.RESETTING   );

	addState(GEMStates.TESTING_TTS           );
	addState(GEMStates.PREPARING_TTSTEST_MODE);

	//
	// Defines the Initial state.
	//
	setInitialState(GEMStates.INITIAL);

	//
	// Defines the Inputs (Commands) for this Finite State Machine.
	//
	addInput(GEMInputs.INITIALIZE);
	addInput(GEMInputs.ENABLE    );
	addInput(GEMInputs.CONFIGURE );
	addInput(GEMInputs.START     );
	addInput(GEMInputs.STOP      );
	addInput(GEMInputs.PAUSE     );
	addInput(GEMInputs.RESUME    );
	addInput(GEMInputs.HALT      );
	addInput(GEMInputs.RECOVER   );
	addInput(GEMInputs.RESET     );
	addInput(GEMInputs.SETERROR  );

	addInput(GEMInputs.TTSTEST_MODE);
	addInput(GEMInputs.TEST_TTS    );
		
	// The SETERROR Input moves the FSM in the ERROR State.
	// This command is not allowed from the GUI.
	// It is instead used inside the FSM callbacks.
	GEMInputs.SETERROR.setVisualizable(false);

	// invisible commands needed for fully asynchronous behaviour
	addInput(GEMInputs.SETENABLE   );
	addInput(GEMInputs.SETCONFIGURE);
	addInput(GEMInputs.SETSTART    );
	addInput(GEMInputs.SETPAUSE    );
	addInput(GEMInputs.SETRESUME   );
	addInput(GEMInputs.SETHALT     );
	addInput(GEMInputs.SETINITIAL  );
	addInput(GEMInputs.SETRESET    );

	addInput(GEMInputs.SETTESTING_TTS);
	addInput(GEMInputs.SETTTSTEST_MODE);
		
		
	// make these invisible
	GEMInputs.SETCONFIGURE.setVisualizable(false);
	GEMInputs.SETSTART.setVisualizable(false);
	GEMInputs.SETPAUSE.setVisualizable(false);
	GEMInputs.SETRESUME.setVisualizable(false);
	GEMInputs.SETHALT.setVisualizable(false);
	GEMInputs.SETINITIAL.setVisualizable(false);
	GEMInputs.SETRESET.setVisualizable(false);
	GEMInputs.SETTTSTEST_MODE.setVisualizable(false);
		
	//
	// Define command parameters.
	// These are then visible in the default GUI.
	//
		
	// define parameters for tts testing command
	//
	CommandParameter<IntegerT> ttsTestFedid          = new CommandParameter<IntegerT>(GEMParameters.TTS_TEST_FED_ID,          new IntegerT(-1));
	CommandParameter<StringT>  ttsTestMode           = new CommandParameter<StringT>( GEMParameters.TTS_TEST_MODE,            new StringT("") );
	CommandParameter<StringT>  ttsTestPattern        = new CommandParameter<StringT>( GEMParameters.TTS_TEST_PATTERN,         new StringT("") );
	CommandParameter<IntegerT> ttsTestSequenceRepeat = new CommandParameter<IntegerT>(GEMParameters.TTS_TEST_SEQUENCE_REPEAT, new IntegerT(-1));
		
	// define parameter set
	ParameterSet<CommandParameter> ttsTestParameters = new ParameterSet<CommandParameter>();
	try {
	    ttsTestParameters.add(ttsTestFedid);
	    ttsTestParameters.add(ttsTestMode);
	    ttsTestParameters.add(ttsTestPattern);
	    ttsTestParameters.add(ttsTestSequenceRepeat);
	} catch (ParameterException nothing) {
	    // Throws an exception if a parameter is duplicate
	    throw new StateMachineDefinitionException( "Could not add to ttsTestParameters. Duplicate Parameter?", nothing );
	}
		
	// set the test parameters
	GEMInputs.TEST_TTS.setParameters(ttsTestParameters);
	
	//
	// define parameters for Initialize command
	//
	CommandParameter<IntegerT> initializeSid = new CommandParameter<IntegerT>(GEMParameters.SID, new IntegerT(0));
	CommandParameter<StringT> initializeGlobalConfigurationKey = new CommandParameter<StringT>(GEMParameters.GLOBAL_CONF_KEY, new StringT(""));

	// define parameter set
	ParameterSet<CommandParameter> initializeParameters = new ParameterSet<CommandParameter>();
	try {
	    initializeParameters.add(initializeSid);
	    initializeParameters.add(initializeGlobalConfigurationKey);
	} catch (ParameterException nothing) {
	    // Throws an exception if a parameter is duplicate
	    throw new StateMachineDefinitionException( "Could not add to initializeParameters. Duplicate Parameter?", nothing );
	}
		
	GEMInputs.INITIALIZE.setParameters(initializeParameters);
	
	//
	// define parameters for Configure command
	//
	CommandParameter<StringT> configureRunType = new CommandParameter<StringT>(GEMParameters.RUN_TYPE, new StringT(""));

	// define parameter set
	ParameterSet<CommandParameter> configureParameters = new ParameterSet<CommandParameter>();
	try {
	    configureParameters.add(configureRunType);
	} catch (ParameterException nothing) {
	    // Throws an exception if a parameter is duplicate
	    throw new StateMachineDefinitionException( "Could not add to configureParameters. Duplicate Parameter?", nothing );
	}
		
	GEMInputs.CONFIGURE.setParameters(configureParameters);
	
	//
	// define parameters for Start command
	//
	CommandParameter<IntegerT> startRunNumber = new CommandParameter<IntegerT>(GEMParameters.RUN_NUMBER, new IntegerT(-1));

	// define parameter set
	ParameterSet<CommandParameter> startParameters = new ParameterSet<CommandParameter>();
	try {
	    startParameters.add(startRunNumber);
	} catch (ParameterException nothing) {
	    // Throws an exception if a parameter is duplicate
	    throw new StateMachineDefinitionException( "Could not add to startParameters. Duplicate Parameter?", nothing );
	}
		
	GEMInputs.START.setParameters(startParameters);
	
		
	//
	// Define the State Transitions
	//

	// INIT Command:
	// The INIT input is allowed only in the INITIAL state, and moves the
	// FSM to the INITIALIZING state.
	//
	addTransition(GEMInputs.INITIALIZE, GEMStates.INITIAL, GEMStates.INITIALIZING);
	
	// TEST_MODE Command:
	// The TEST_MODE input is allowed in the HALTED state and moves 
	// the FSM to the PREPARING_TEST_MODE state.
	//
	addTransition(GEMInputs.TTSTEST_MODE, GEMStates.HALTED, GEMStates.PREPARING_TTSTEST_MODE);

	// Reach the TEST_MODE State
	addTransition(GEMInputs.SETTTSTEST_MODE, GEMStates.PREPARING_TTSTEST_MODE, GEMStates.TTSTEST_MODE);
	addTransition(GEMInputs.SETTTSTEST_MODE, GEMStates.TESTING_TTS,            GEMStates.TTSTEST_MODE);

	// TEST_TTS Command:
	// The TEST_TTS input is allowed in the TEST_MODE state and moves
	// the FSM to the TESTING_TTS state.
	addTransition(GEMInputs.TEST_TTS, GEMStates.TTSTEST_MODE, GEMStates.TESTING_TTS);

	// CONFIGURE Command:
	// The CONFIGURE input is allowed only in the HALTED state, and moves
	// the FSM to the CONFIGURING state.
	//
	addTransition(GEMInputs.CONFIGURE, GEMStates.HALTED, GEMStates.CONFIGURING);

	// START Command:
	// The START input is allowed only in the CONFIGURED state, and moves
	// the FSM to the STARTING state.
	//
	addTransition(GEMInputs.START, GEMStates.CONFIGURED, GEMStates.STARTING);

	// PAUSE Command:
	// The PAUSE input is allowed only in the RUNNING state, and moves
	// the FSM to the PAUSING state.
	//
	addTransition(GEMInputs.PAUSE, GEMStates.RUNNING, GEMStates.PAUSING);

	// RESUME Command:
	// The RESUME input is allowed only in the PAUSED state, and moves
	// the FSM to the RESUMING state.
	//
	addTransition(GEMInputs.RESUME, GEMStates.PAUSED, GEMStates.RESUMING);
		
	// STOP Command:
	// The STOP input is allowed only in the RUNNING and PAUSED state, and moves
	// the FSM to the CONFIGURED state.
	//
	addTransition(GEMInputs.STOP, GEMStates.RUNNING, GEMStates.STOPPING);
	addTransition(GEMInputs.STOP, GEMStates.PAUSED,  GEMStates.STOPPING);
		
	// HALT Command:
	// The HALT input is allowed in the RUNNING, CONFIGURED and PAUSED
	// state, and moves the FSM to the HALTING state.
	//
	addTransition(GEMInputs.HALT, GEMStates.RUNNING,      GEMStates.HALTING);
	addTransition(GEMInputs.HALT, GEMStates.CONFIGURED,   GEMStates.HALTING);
	addTransition(GEMInputs.HALT, GEMStates.PAUSED,       GEMStates.HALTING);
	addTransition(GEMInputs.HALT, GEMStates.TTSTEST_MODE, GEMStates.HALTING);

	// RECOVER Command:
	// The RECOVER input is allowed from ERROR and moves the FSM in to
	// RECOVERING state.
	//
	addTransition(GEMInputs.RECOVER, GEMStates.ERROR, GEMStates.RECOVERING);

	// RESET Command:
	// The RESET input is allowed from any steady state and moves the FSM to the
	// RESETTING state.
	//
	addTransition(GEMInputs.RESET, GEMStates.HALTED,       GEMStates.RESETTING);
	addTransition(GEMInputs.RESET, GEMStates.CONFIGURED,   GEMStates.RESETTING);
	addTransition(GEMInputs.RESET, GEMStates.RUNNING,      GEMStates.RESETTING);
	addTransition(GEMInputs.RESET, GEMStates.PAUSED,       GEMStates.RESETTING);
	addTransition(GEMInputs.RESET, GEMStates.TTSTEST_MODE, GEMStates.RESETTING);
	addTransition(GEMInputs.RESET, GEMStates.ERROR,        GEMStates.RESETTING);		

	//
	// The following transitions are not triggered from the GUI.
	//

	// Transition for going to ERROR
	addTransition(GEMInputs.SETERROR, State.ANYSTATE, GEMStates.ERROR);

	//
	// add transitions for transitional States
	//

	// Reach the INITIAL State
	addTransition(GEMInputs.SETINITIAL, GEMStates.RECOVERING, GEMStates.INITIAL);

	// Reach the HALTED State
	addTransition(GEMInputs.SETHALT, GEMStates.INITIALIZING, GEMStates.HALTED);
	addTransition(GEMInputs.SETHALT, GEMStates.HALTING,      GEMStates.HALTED);
	addTransition(GEMInputs.SETHALT, GEMStates.RECOVERING,   GEMStates.HALTED);
	addTransition(GEMInputs.SETHALT, GEMStates.RESETTING,    GEMStates.HALTED);

	// Reach the CONFIGURED State
	addTransition(GEMInputs.SETCONFIGURE, GEMStates.INITIALIZING,
		      GEMStates.CONFIGURED);
	addTransition(GEMInputs.SETCONFIGURE, GEMStates.RECOVERING,
		      GEMStates.CONFIGURED);
	addTransition(GEMInputs.SETCONFIGURE, GEMStates.CONFIGURING,
		      GEMStates.CONFIGURED);
	addTransition(GEMInputs.SETCONFIGURE, GEMStates.STOPPING,
		      GEMStates.CONFIGURED);
		
	// Reach the RUNNING State
	addTransition(GEMInputs.SETSTART, GEMStates.INITIALIZING,
		      GEMStates.RUNNING);
	addTransition(GEMInputs.SETSTART, GEMStates.RECOVERING, GEMStates.RUNNING);
	addTransition(GEMInputs.SETSTART, GEMStates.STARTING,   GEMStates.RUNNING);

	// Reach the PAUSED State
	addTransition(GEMInputs.SETPAUSE, GEMStates.PAUSING,    GEMStates.PAUSED);
	addTransition(GEMInputs.SETPAUSE, GEMStates.RECOVERING, GEMStates.PAUSED);

	// Reach the RUNNING from RESUMING State
	addTransition(GEMInputs.SETRESUME, GEMStates.RESUMING, GEMStates.RUNNING);
    }

}