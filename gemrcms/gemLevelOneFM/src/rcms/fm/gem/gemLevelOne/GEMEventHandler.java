package rcms.fm.gem.gemLevelOne;

import static java.util.concurrent.TimeUnit.MILLISECONDS;

import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;

import rcms.errorFormat.CMS.CMSError;
import rcms.fm.fw.EventHandlerException;
import rcms.fm.fw.StateEnteredEvent;
import rcms.fm.fw.parameter.CommandParameter;
import rcms.fm.fw.parameter.FunctionManagerParameter;
import rcms.fm.fw.parameter.ParameterException;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.parameter.type.IntegerT;
import rcms.fm.fw.parameter.type.StringT;
import rcms.fm.fw.user.UserActionException;
import rcms.fm.fw.user.UserStateNotificationHandler;
import rcms.fm.resource.QualifiedGroup;
import rcms.fm.resource.QualifiedResource;
import rcms.fm.resource.QualifiedResourceContainerException;
import rcms.fm.resource.qualifiedresource.JobControl;
import rcms.fm.resource.qualifiedresource.XdaqApplication;
import rcms.fm.resource.qualifiedresource.XdaqApplicationContainer;
import rcms.fm.resource.qualifiedresource.XdaqExecutive;
import rcms.stateFormat.StateNotification;
import rcms.statemachine.definition.Input;
import rcms.util.logger.RCMSLogger;
import rcms.xdaqctl.XDAQParameter;

// needed to store RunInfo
import rcms.utilities.runinfo.RunInfo;
import rcms.utilities.runinfo.RunInfoConnectorIF;
import rcms.utilities.runinfo.RunInfoException;
import rcms.utilities.runinfo.RunNumberData;
import rcms.utilities.runinfo.RunSequenceNumber;
import rcms.fm.fw.parameter.Parameter;

import rcms.fm.gem.gemLevelOne.util.GEMUtil;
/**
 * 
 * Main Event Handler class for GEM Level 1 Function Manager.
 * 
 * @author Andrea Petrucci, Alexander Oh, Michele Gulmini
 * Adapted for GEM by Jared Sturdy
 */
public class GEMEventHandler extends UserStateNotificationHandler {

    /**
     * <code>RCMSLogger</code>: RCMS log4j logger.
     */
    static RCMSLogger logger = new RCMSLogger(GEMEventHandler.class);

    GEMFunctionManager functionManager = null;

    private QualifiedGroup qualifiedGroup  = null;
    private RunNumberData _myRunNumberData = null;

    // parameters for tts test
    private XDAQParameter   svTTSParameter  = null;
    private TTSSetter       ttsSetter       = null;
    private ScheduledFuture ttsSetterFuture = null;
    private final ScheduledExecutorService scheduler;

    protected RunSequenceNumber runNumberGenerator = null;
    protected RunNumberData     runNumberData      = null;
    protected Integer           runNumber          = null;
    protected boolean           localRunNumber     = false;
    protected RunInfo           runInfo            = null;


    /**
     * Handle to utility class
     */
    private GEMUtil _gemUtil;
	
    public GEMEventHandler() throws rcms.fm.fw.EventHandlerException {
	// this handler inherits UserStateNotificationHandler
	// so it is already registered for StateNotification events

	// Let's register also the StateEnteredEvent triggered when the FSM enters in a new state.
	subscribeForEvents(StateEnteredEvent.class);

	addAction(GEMStates.INITIALIZING,	    "initAction"     );
	addAction(GEMStates.CONFIGURING, 	    "configureAction");
	addAction(GEMStates.HALTING,     	    "haltAction"     );
	addAction(GEMStates.PREPARING_TTSTEST_MODE, "preparingTTSTestModeAction");
	addAction(GEMStates.TESTING_TTS,    	    "testingTTSAction");
	addAction(GEMStates.PAUSING,     	    "pauseAction"  );
	addAction(GEMStates.RECOVERING,  	    "recoverAction");
	addAction(GEMStates.RESETTING,   	    "resetAction"  );
	addAction(GEMStates.RESUMING,    	    "resumeAction" );
	addAction(GEMStates.STARTING,    	    "startAction"  );
	addAction(GEMStates.STOPPING,    	    "stopAction"   );

	scheduler = Executors.newScheduledThreadPool(1);
    }


    public void init() throws rcms.fm.fw.EventHandlerException {
	functionManager = (GEMFunctionManager) getUserFunctionManager();
	qualifiedGroup  = functionManager.getQualifiedGroup();
	//Unique id for each configuration 
	
	int session_id = functionManager.getQualifiedGroup().getGroup().getDirectory().getId();
	functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.SID, new IntegerT(session_id)));
	
	// instantiate utility
	_gemUtil = new GEMUtil(functionManager);
	
	ttsSetter = new TTSSetter();
	ttsSetterFuture = null;
	
	// debug
	logger.debug("GEM levelOneFM init() called: functionManager=" + functionManager );
	// call renderers
	//_gemUtil.renderMainGui();
    }

    public void initAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    // triggered by State Notification from child resource

	    /************************************************
	     * PUT YOUR CODE HERE
	     ***********************************************/

	    return;
	}

	else if (obj instanceof StateEnteredEvent) {

	    // triggered by entered state action
	    // let's command the child resources
	    //_gemUtil.setParameter(action ,"Initializing" );

	    // debug
	    logger.debug("initAction called.");

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,
                                                                                        new StringT("Initializing")));

	    // update state
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.STATE,
                                                                                        new StringT(functionManager.getState().getStateString())));

	    // initialize qualified group
	    try {
		qualifiedGroup.init();
	    } catch (Exception e) {
		// failed to init
		String errMsg = this.getClass().toString() + " failed to initialize resources";

		// send error notification
		sendCMSError(errMsg);

		//log error
		logger.error(errMsg,e);

		// go to error state
		functionManager.fireEvent(GEMInputs.SETERROR);
	    }

	    // find xdaq applications
	    List<QualifiedResource> xdaqList = qualifiedGroup.seekQualifiedResourcesOfType(new XdaqApplication());
	    functionManager.containerXdaqApplication = new XdaqApplicationContainer(xdaqList);
	    logger.debug("Application list : " + xdaqList.size() );

	    // set paraterts from properties
	    _gemUtil.setParameterFromProperties();
			
	    // render gui
	    _gemUtil.renderMainGui();
			
	    functionManager.gemSupervisor = 
		new XdaqApplicationContainer(functionManager.containerXdaqApplication.getApplicationsOfClass(
                                                                                                             "gem::supervisor::Application"));

	    // go to HALT
	    functionManager.fireEvent( GEMInputs.SETHALT );


	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("")));


	    // parameters for cross checks (RCMS task #12155)
	    ParameterSet parameters = getUserFunctionManager().getLastInput().getParameterSet();
	    IntegerT sid             = (IntegerT)parameters.get(GEMParameters.SID            ).getValue();
	    functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.SID, sid));
	    StringT  global_conf_key = (StringT)parameters.get(GEMParameters.GLOBAL_CONF_KEY).getValue();
	    functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.INITIALIZED_WITH_SID            , sid             ));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.INITIALIZED_WITH_GLOBAL_CONF_KEY, global_conf_key ));


	    logger.info("initAction Executed");
	}
    }


    public void resetAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    // triggered by State Notification from child resource

	    /************************************************
	     * PUT HERE YOUR CODE							
	     ***********************************************/

	    return;
	}

	else if (obj instanceof StateEnteredEvent) {


	    // triggered by entered state action
	    // let's command the child resources

	    // debug
	    logger.debug("resetAction called.");

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("Resetting")));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.STATE, new StringT(
															 functionManager.getState().getStateString())));

	    // destroy xdaq
	    destroyXDAQ();

	    // reset qualified group
	    QualifiedGroup qualifiedGroup = functionManager.getQualifiedGroup();
	    qualifiedGroup.reset();

	    // reinitialize qualified group
	    try {
		qualifiedGroup.init();
	    }
	    catch (Exception e) {
		String errorMess = e.getMessage();
		logger.error(errorMess);
		throw new UserActionException(errorMess);
	    }

	    // go to Initital
	    functionManager.fireEvent( GEMInputs.SETHALT );

	    // Clean-up of the Function Manager parameters
	    cleanUpFMParameters();

	    logger.info("resetAction Executed");
	}	
    }

    public void recoverAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    // triggered by State Notification from child resource

	    /************************************************
	     * PUT HERE YOUR CODE							
	     ***********************************************/

	    return;
	}

	else if (obj instanceof StateEnteredEvent) {

	    System.out.println("Executing recoverAction");
	    logger.info("Executing recoverAction");

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("recovering")));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(
											GEMParameters.STATE,new StringT(functionManager.getState().getStateString())));

	    /************************************************
	     * PUT HERE YOUR CODE							
	     ***********************************************/

	    // leave intermediate state
	    functionManager.fireEvent( GEMInputs.SETINITIAL );

	    // Clean-up of the Function Manager parameters
	    cleanUpFMParameters();

	    logger.info("recoverAction Executed");
	}
    }

    public void configureAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    // triggered by State Notification from child resource

	    // leave intermediate state
	    // check that the gem supervisor is in configured state
	    for ( XdaqApplication xdaqApp : functionManager.gemSupervisor.getApplications()) {
		if (xdaqApp.getCacheState().equals(GEMStates.ERROR)) {
		    functionManager.fireEvent(GEMInputs.SETERROR);
		}
		else if (!xdaqApp.getCacheState().equals(GEMStates.CONFIGURED)) return;
	    }
	    functionManager.fireEvent( GEMInputs.SETCONFIGURE );

	    return;
	}

	else if (obj instanceof StateEnteredEvent) {
	    System.out.println("Executing configureAction");
	    logger.info("Executing configureAction");


	    // check that we have a gem supervisor to control
	    if (functionManager.gemSupervisor.getApplications().size() == 0) {
		// nothing to control, go to configured immediately
		functionManager.fireEvent( GEMInputs.SETCONFIGURE );
		return;
	    }

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("configuring")));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.STATE, new StringT("Configured")));

	    //get the parameters of the command
	    ParameterSet<CommandParameter> parameterSet = getUserFunctionManager().getLastInput().getParameterSet();

	    // check parameter set
	    if (parameterSet.size()==0 || parameterSet.get(GEMParameters.RUN_TYPE) == null ||
		((StringT)parameterSet.get(GEMParameters.RUN_TYPE).getValue()).equals("") )  {

		// Set default
		try {
		    parameterSet.add( new CommandParameter<StringT>(GEMParameters.RUN_TYPE, new StringT("Default")));
		} catch (ParameterException e) {
		    logger.error("Could not default the run type to Default",e);
		    functionManager.fireEvent(GEMInputs.SETERROR);
		}
	    }

	    // get the run number from the configure command
	    // get the calib key and put it as run type (another ichiro special)
	    String runType = ((StringT)functionManager.getParameterSet().get(GEMParameters.GEM_CALIB_KEY).getValue()).getString();

	    // Set the runType in the Function Manager parameters
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.RUN_TYPE,new StringT(runType)));


	    // set run type parameter			
	    try {
		XDAQParameter xdaqParam = ((XdaqApplication)
					   functionManager.gemSupervisor.getApplications().get(0))
		    .getXDAQParameter();

		// select RunType of gem supervisor.
		// this parameter is used to differentiate between 
		// 1) calibration run
		// 2) global run
		// since level one is only used in global runs we hardwire
		// for the time being to global run = "Default".
		// the parameter is set to the supervisor xdaq application.
		xdaqParam.select("RunType");
		xdaqParam.setValue("RunType", runType);
		xdaqParam.send();
				

	    } catch (Exception e) {
		logger.error(getClass().toString() +
			     "Failed to set run type Default to gem supervisor xdaq application. (Value is hardwired in the code) ", e);

		functionManager.fireEvent(GEMInputs.SETERROR);
	    }

	    // send Configure
	    try {
		functionManager.gemSupervisor.execute(GEMInputs.CONFIGURE);

	    } catch (Exception e) {
		logger.error(getClass().toString() +
			     "Failed to Configure gem supervisor xdaq application.", e);

		functionManager.fireEvent(GEMInputs.SETERROR);
	    }

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("")));

	    // parameters for cross checks (RCMS task #12155)
	    ParameterSet parameters = getUserFunctionManager().getLastInput().getParameterSet();
	    IntegerT run_number      = (IntegerT)parameters.get(GEMParameters.RUN_NUMBER     ).getValue();
	    StringT  run_key         = (StringT) parameters.get(GEMParameters.RUN_KEY        ).getValue();
	    StringT  tpg_key         = (StringT) parameters.get(GEMParameters.TPG_KEY        ).getValue();
	    StringT  fed_enable_mask = (StringT) parameters.get(GEMParameters.FED_ENABLE_MASK).getValue();
	    functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.CONFIGURED_WITH_RUN_NUMBER     , run_number));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT> (GEMParameters.CONFIGURED_WITH_RUN_KEY        , run_key));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT> (GEMParameters.CONFIGURED_WITH_TPG_KEY        , tpg_key));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT> (GEMParameters.CONFIGURED_WITH_FED_ENABLE_MASK, fed_enable_mask));


	    logger.info("configureAction Executed");
	}
    }

    public void startAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    // triggered by State Notification from child resource

	    // leave intermediate state
	    // check that the gem supervisor is in configured state
	    for ( XdaqApplication xdaqApp : functionManager.gemSupervisor.getApplications()) {
		if (xdaqApp.getCacheState().equals(GEMStates.ERROR)) {
		    functionManager.fireEvent(GEMInputs.SETERROR);
		}
		else if (xdaqApp.getCacheState().equals(GEMStates.ERROR)) {
		    functionManager.fireEvent(GEMInputs.SETERROR);
		}
		else if (!xdaqApp.getCacheState().equals(GEMStates.RUNNING)) return;
	    }
	    functionManager.fireEvent( GEMInputs.SETSTART );

	    return;
	}

	else if (obj instanceof StateEnteredEvent) {
	    System.out.println("Executing startAction");
	    logger.info("Executing startAction");


	    // check that we have a gem supervisor to control
	    if (functionManager.gemSupervisor.getApplications().size() == 0) {
		// nothing to control, go to configured immediately
		functionManager.fireEvent( GEMInputs.SETSTART );
		return;
	    }

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("starting")));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.STATE, new StringT("Running")));

	    // get the parameters of the command
	    ParameterSet<CommandParameter> parameterSet = getUserFunctionManager().getLastInput().getParameterSet();
	    runNumber = null;
	    localRunNumber = false;


	    // check parameter set
	    if (parameterSet.size()==0 || parameterSet.get(GEMParameters.RUN_NUMBER) == null )  {
		// go to error, we require parameters
		String errMsg = "startAction: no parameters given with start command";
		
		// log remark
		logger.info(errMsg);
	
		RunInfoConnectorIF ric = functionManager.getRunInfoConnector();
		if ( ric != null )
		    {
			if ( runNumberGenerator == null )
			    {
				logger.info("Creating RunNumberGenerator (RunSequenceNumber) ");
				runNumberGenerator = new RunSequenceNumber(ric,functionManager.getOwner(),functionManager.getParameterSet().
									   get(GEMParameters.SEQ_NAME).getValue().toString());
			    }

			if ( runNumberGenerator != null )
			    {
				runNumberData = runNumberGenerator.createRunSequenceNumber(new Integer(functionManager.getParameterSet().get(GEMParameters.SID).
												       getValue().toString()));
			    }

			if ( runNumberData != null )
			    {
				runNumber =  runNumberData.getRunNumber();
				System.out.println("Generated RunNumber\n"+runNumberData.toString());
				localRunNumber = true;
			    }
			else
			    {
				logger.error("Error generating RunNumber");
			    }
		    }
		else
		    {
			logger.error("Failed to create RunNumberGenerator");
		    }
	    }
	    else
		{
		    // get the run number from the start command
		    runNumber =((IntegerT)parameterSet.get(GEMParameters.RUN_NUMBER).getValue()).getInteger();
		}
      
	    if ( runNumber != null )
		{
		    // Set the run number in the Function Manager parameters
		    functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.RUN_NUMBER,new IntegerT(runNumber)));
		}
	    else
		{
		    logger.error("Cannot find and/or generator RunNumber");
		}
    
	    publishRunInfo(true);


	    /*    
	    // default to -1
	    try {
	    parameterSet.add( new CommandParameter<IntegerT>(GEMParameters.RUN_NUMBER, new IntegerT(-1)));
	    } catch (ParameterException e) {
	    logger.error("Could not default the run number to -1",e);
	    functionManager.fireEvent(GEMInputs.SETERROR);
	    }
		    
	    // log this
	    logger.warn("No run number given, defaulting to -1.");
	    */
		    
		
		
	    // get the run number from the start command
	    //Integer runNumber = ((IntegerT)parameterSet.get(GEMParameters.RUN_NUMBER).getValue()).getInteger();
		
	    // Set the run number in the Function Manager parameters
	    //functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.RUN_NUMBER,new IntegerT(-1)));
		
	    //			set run number parameter
	    try {
		XDAQParameter xdaqParam = ((XdaqApplication)
					   functionManager.gemSupervisor.getApplications().get(0)).getXDAQParameter();
		    
		xdaqParam.select("RunNumber");
		ParameterSet<CommandParameter> commandParam = getUserFunctionManager().getLastInput().getParameterSet();
		if (commandParam == null) {
		    logger.error(getClass().toString() +"Failded to Enable XDAQ, no run # specified.");
		    functionManager.fireEvent(GEMInputs.SETERROR);
		}
		logger.debug(getClass().toString() + "Run #: " + runNumber);
		    
		xdaqParam.setValue("RunNumber", runNumber.toString());
		xdaqParam.send();
		    
	    } catch (Exception e) {
		logger.error(getClass().toString() + "Failed to Enable gem supervisor XDAQ application.", e);
		    
		functionManager.fireEvent(GEMInputs.SETERROR);
	    }
		
	    // send Enable
	    try {
		functionManager.gemSupervisor.execute(new Input("Start"));
			    
	    } catch (Exception e) {
		logger.error(getClass().toString() + "Failed to Enable gem supervisor XDAQ application.", e);
			    
		functionManager.fireEvent(GEMInputs.SETERROR);
	    }
			
	    logger.debug("GEMLeadingActions.start ... done.");
			
			
	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("")));
			
	    // parameters for cross checks (RCMS task #12155)
	    ParameterSet parameters = getUserFunctionManager().getLastInput().getParameterSet();
	    IntegerT run_number      = (IntegerT)parameters.get(GEMParameters.RUN_NUMBER).getValue();
	    functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.STARTED_WITH_RUN_NUMBER, run_number));


	    logger.debug("startAction Executed");
			
	}
    }
    
    public void pauseAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    // triggered by State Notification from child resource

	    /************************************************
	     * PUT HERE YOUR CODE							
	     ***********************************************/

	    return;
	}

	else if (obj instanceof StateEnteredEvent) {	
	    System.out.println("Executing pauseAction");
	    logger.info("Executing pauseAction");

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("pausing")));

	    /************************************************
	     * PUT HERE YOUR CODE							
	     ***********************************************/

	    // leave intermediate state
	    functionManager.fireEvent( GEMInputs.SETPAUSE );

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("")));
			
	    logger.debug("pausingAction Executed");
			
	}
    }

    public void stopAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    // triggered by State Notification from child resource

	    // leave intermediate state
	    // check that the gem supervisor is in configured state
	    for ( XdaqApplication xdaqApp : functionManager.gemSupervisor.getApplications()) {
		if (xdaqApp.getCacheState().equals(GEMStates.ERROR)) {
		    functionManager.fireEvent(GEMInputs.SETERROR);
		}
		else if (!xdaqApp.getCacheState().equals(GEMStates.CONFIGURED)) return;
	    }
	    functionManager.fireEvent( GEMInputs.SETCONFIGURE );

	    return;
	}

	else if (obj instanceof StateEnteredEvent) {	
	    System.out.println("Executing stopAction");
	    logger.info("Executing stopAction");

	    // check that we have a gem supervisor to control
	    if (functionManager.gemSupervisor.getApplications().size() == 0) {
		// nothing to control, go to configured immediately
		functionManager.fireEvent( GEMInputs.SETCONFIGURE );
		return;
	    }

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("stopping")));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.STATE, new StringT("Halted")));
			
	    //Run Info
	    publishRunInfo(false);

	    // send Stop
	    try {
		functionManager.gemSupervisor.execute(new Input("Stop"));

	    } catch (Exception e) {
		logger.error(getClass().toString() + "Failed to Stop XDAQ.", e);

		functionManager.fireEvent(GEMInputs.SETERROR);
	    }


	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("")));
			
	    logger.debug("stopAction Executed");
			
	}
    }
    public void resumeAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    // triggered by State Notification from child resource

	    /************************************************
	     * PUT HERE YOUR CODE							
	     ***********************************************/

	    return;
	}

	else if (obj instanceof StateEnteredEvent) {	
	    System.out.println("Executing resumeAction");
	    logger.info("Executing resumeAction");

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("resuming")));

	    /************************************************
	     * PUT HERE YOUR CODE							
	     ***********************************************/

	    // leave intermediate state
	    functionManager.fireEvent( GEMInputs.SETRESUME );

	    // Clean-up of the Function Manager parameters
	    cleanUpFMParameters();

	    logger.debug("resumeAction Executed");

	}
    }

    public void haltAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    //			triggered by State Notification from child resource

	    // leave intermediate state
	    // check that the gem supervisor is in configured state
	    for ( XdaqApplication xdaqApp : functionManager.gemSupervisor.getApplications()) {
		if (xdaqApp.getCacheState().equals(GEMStates.ERROR)) {
		    functionManager.fireEvent(GEMInputs.SETERROR);
		}
		else if (!xdaqApp.getCacheState().equals(GEMStates.HALTED)) return;
	    }
	    functionManager.fireEvent( GEMInputs.SETHALT );

	    return;
	}

	else if (obj instanceof StateEnteredEvent) {
	    System.out.println("Executing haltAction");
	    logger.info("Executing haltAction");

	    // check that we have a gem supervisor to control
	    if (functionManager.gemSupervisor.getApplications().size() == 0) {
		// nothing to control, go to configured immediately
		functionManager.fireEvent( GEMInputs.SETHALT );
		return;
	    }

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("halting")));

	    // send Halt
	    try {
		functionManager.gemSupervisor.execute(new Input("Halt"));

	    } catch (Exception e) {
		logger.error(getClass().toString() + "Failed to Halt XDAQ.", e);

		functionManager.fireEvent(GEMInputs.SETERROR);
	    }


	    // check from which state we came.
	    if (functionManager.getPreviousState().equals(GEMStates.TTSTEST_MODE)) {
		// when we came from TTSTestMode we need to
		// 1. give back control of sTTS to HW
	    }


	    // Clean-up of the Function Manager parameters
	    cleanUpFMParameters();

	    logger.debug("haltAction Executed");
	}
    }	

    public void preparingTTSTestModeAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    // triggered by State Notification from child resource

	    StateNotification sn = (StateNotification)obj;
			
	    if (sn.getToState().equals(GEMStates.CONFIGURED.getStateString())) {
		// configure is send in state entered part (below)
		// here we receive "Configured" from supervisor.
		// send a enable now.
		try {                                                                                           
		    functionManager.gemSupervisor.execute(new Input("Start"));
		} catch (QualifiedResourceContainerException e) {
		    logger.error("Could not enable gem supervisor.",e);
		    functionManager.fireEvent(GEMInputs.SETERROR);
		}
	    }
	    else if ( sn.getToState().equals(GEMStates.RUNNING.getStateString()) ) {
		// recveived runnning state. now we can
		// leave intermediate state
		functionManager.fireEvent( GEMInputs.SETTTSTEST_MODE );
	    }
	    else {
		// don't understand state, go to error.
		logger.error("Unexpected state notification in state " 
			     + functionManager.getState().toString() 
			     + ", received state :" 
			     + sn.getToState());
		functionManager.fireEvent(GEMInputs.SETERROR);
	    }
	    return;
	}

	else if (obj instanceof StateEnteredEvent) {
	    System.out.println("Executing preparingTestModeAction");
	    logger.info("Executing preparingTestModeAction");

	    // check that we have a gem supervisor to control
	    if (functionManager.gemSupervisor.getApplications().size() == 0) {
		// nothing to control, go to configured immediately
		functionManager.fireEvent( GEMInputs.SETTTSTEST_MODE );
		return;
	    }

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("preparingTestMode")));

	    // set run type parameter
	    try {
		XDAQParameter xdaqParam = ((XdaqApplication)
					   functionManager.gemSupervisor.getApplications().get(0))
		    .getXDAQParameter();

		xdaqParam.select("RunType");
		xdaqParam.setValue("RunType", "sTTS_Test");
		xdaqParam.send();

	    } catch (Exception e) {
		logger.error(getClass().toString() +
			     "Failed to set run type: " + "sTTS_Test", e);

		functionManager.fireEvent(GEMInputs.SETERROR);
	    }

	    try {
		functionManager.gemSupervisor.execute(new Input("Configure"));


	    } catch (Exception e) {
		logger.error(getClass().toString() +
			     "Failed to TTSPrepare XDAQ.", e);

		functionManager.fireEvent(GEMInputs.SETERROR);
	    }

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("")));

	    logger.debug("preparingTestModeAction Executed");
	}
    }	

    public void testingTTSAction(Object obj) throws UserActionException {

	if (obj instanceof StateNotification) {

	    // triggered by State Notification from child resource

	    /************************************************
	     * PUT HERE YOUR CODE							
	     ***********************************************/

	    return;
	}

	else if (obj instanceof StateEnteredEvent) {
	    System.out.println("Executing testingTTSAction");
	    logger.info("Executing testingTTSAction");

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("testing TTS")));

	    // get the parameters of the command
	    ParameterSet<CommandParameter> parameterSet = getUserFunctionManager().getLastInput().getParameterSet();

	    // check parameter set
	    if (parameterSet.size()==0 || parameterSet.get(GEMParameters.TTS_TEST_FED_ID) == null ||
		parameterSet.get(GEMParameters.TTS_TEST_MODE) == null ||
		((StringT)parameterSet.get(GEMParameters.TTS_TEST_MODE).getValue()).equals("") || 
		parameterSet.get(GEMParameters.TTS_TEST_PATTERN) == null ||
		((StringT)parameterSet.get(GEMParameters.TTS_TEST_PATTERN).getValue()).equals("") ||
		parameterSet.get(GEMParameters.TTS_TEST_SEQUENCE_REPEAT) == null)
		{

		    // go to error, we require parameters
		    String errMsg = "testingTTSAction: no parameters given with TestTTS command.";

		    // log error
		    logger.error(errMsg);

		    // notify error
		    sendCMSError(errMsg);

		    //go to error state
		    functionManager.fireEvent( GEMInputs.SETERROR );

		}

	    Integer fedId = ((IntegerT)parameterSet.get(GEMParameters.TTS_TEST_FED_ID).getValue()).getInteger();
	    String mode = ((StringT)parameterSet.get(GEMParameters.TTS_TEST_MODE).getValue()).getString();
	    String pattern = ((StringT)parameterSet.get(GEMParameters.TTS_TEST_PATTERN).getValue()).getString();
	    Integer cycles = ((IntegerT)parameterSet.get(GEMParameters.TTS_TEST_SEQUENCE_REPEAT).getValue()).getInteger();


	    // Set last parameters in the Function Manager parameters
	    functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.TTS_TEST_FED_ID,new IntegerT(fedId)));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.TTS_TEST_MODE,new StringT(mode)));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.TTS_TEST_PATTERN,new StringT(pattern)));
	    functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.TTS_TEST_SEQUENCE_REPEAT,new IntegerT(cycles)));

	    // debug
	    logger.debug("Using parameters: fedId=" + fedId + "mode=" + mode + " pattern=" + pattern + " cycles=" + cycles );

	    // find out which application controls the fedId.

	    int crate, slot, bits, repeat;
		
	    int fedID = ((IntegerT)parameterSet.get(GEMParameters.TTS_TEST_FED_ID).getValue()).getInteger();
	    crate = getCrateNumber(fedID);
	    slot = getSlotNumber(fedID);
	    bits = Integer.parseInt(
				    ((StringT)parameterSet.get(
							       GEMParameters.TTS_TEST_PATTERN)
				     .getValue()).getString());
	    if (((StringT)parameterSet.get(GEMParameters.TTS_TEST_PATTERN)
		 .getValue()).getString().equals("PATTERN")) {
		repeat = 0;
	    } else {  // CYCLE
		repeat = ((IntegerT)parameterSet.get(
						     GEMParameters.TTS_TEST_SEQUENCE_REPEAT)
			  .getValue()).getInteger();
	    }

	    ttsSetter.config(crate, slot, bits, repeat);
	    if (ttsSetterFuture == null) {
		ttsSetterFuture = scheduler.scheduleWithFixedDelay(
								   ttsSetter, 0, 10, MILLISECONDS);
	    }


	    // leave intermediate state
	    functionManager.fireEvent( GEMInputs.SETTTSTEST_MODE );

	    // set action
	    functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("")));

	    logger.debug("preparingTestModeAction Executed");
	}
    }	

    @SuppressWarnings("unchecked")
	private void sendCMSError(String errMessage){

	// create a new error notification msg
	CMSError error = functionManager.getErrorFactory().getCMSError();
	error.setDateTime(new Date().toString());
	error.setMessage(errMessage);

	// update error msg parameter for GUI
	functionManager.getParameterSet().get(GEMParameters.ERROR_MSG).setValue(new StringT(errMessage));

	// send error
	try {
	    functionManager.getParentErrorNotifier().sendError(error);
	} catch (Exception e) {
	    logger.warn(functionManager.getClass().toString() + ": Failed to send error mesage " + errMessage);
	}
    }

    private void cleanUpFMParameters() {
	// Clean-up of the Function Manager parameters
	functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ACTION_MSG,new StringT("")));
	functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.ERROR_MSG,new StringT("")));
	functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.RUN_NUMBER,new IntegerT(-1)));
	functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.RUN_TYPE,new StringT("")));
	functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.TTS_TEST_FED_ID,new IntegerT(-1)));
	functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.TTS_TEST_MODE,new StringT("")));
	functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.TTS_TEST_PATTERN,new StringT("")));
	functionManager.getParameterSet().put(new FunctionManagerParameter<IntegerT>(GEMParameters.TTS_TEST_SEQUENCE_REPEAT,new IntegerT(-1)));
    }

    public void publishRunInfo(boolean doRun)
    {
	RunInfoConnectorIF ric = functionManager.getRunInfoConnector();

	if ( ric != null )
	    {
		if ( runNumberData != null )
		    {
			runInfo = new RunInfo(ric,runNumberData);
		    }
		else
		    {
			runInfo = new RunInfo(ric,runNumber);
		    }

		runInfo.setNameSpace(GEMParameters.GEM_NS);
		if ( localRunNumber )
		    {
			try
			    {
				runInfo.publishRunInfo("RunNumber", runNumber.toString());
			    }
			catch ( RunInfoException e )
			    {
				logger.error("RunInfoException: "+e.toString());
			    }
		    }
	    }
    }
    // Record LhcStatus info
    /*
      try
      {
      lhcStatus = LhcStatus.fetchLhcStatus();
      Class c = lhcStatus.getClass();
      Field fields [] = c.getFields();
      for ( int i=0; i<fields.length; i++)
      {
      String fieldName = fields[i].getName();
      Object value = fields[i].get(lhcStatus);
      String type = fields[i].getType().getName();

      if ( type.equals("java.lang.String") )
      {
      StringT sValue = new StringT((String)value);
      Parameter<StringT> p = new Parameter<StringT>(fieldName,sValue);
      runInfo.publishWithHistory(p);
      }
      else if ( type.equals("int" ) )
      {
      IntegerT iValue = new IntegerT((Integer)value);
      Parameter<IntegerT> p = new Parameter<IntegerT>(fieldName,iValue);
      runInfo.publishWithHistory(p);
      }
      }
      }
     
      catch ( Exception e )
      {
      logger.warn(e.toString());
      }
      }
      else

      {
      logger.error("Cannot find RunInfoConnectorIF");
      }
      }
    */

    private void destroyXDAQ() throws UserActionException {
	functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.STATE,new StringT(functionManager.getState().getStateString())));
	
	// get executives and their job controls 
	List<QualifiedResource> listExecutive = qualifiedGroup
	    .seekQualifiedResourcesOfType(new XdaqExecutive());


	for (QualifiedResource ex : listExecutive) {
	    ((XdaqExecutive)ex).killMe();
	    logger.debug("killMe() called on " + ex.getResource().getURL());
	}
    }

    private void killAllXDAQ() throws UserActionException {

	// get executives and their job controls 
	List<QualifiedResource> listJC = qualifiedGroup
	    .seekQualifiedResourcesOfType(new JobControl());

	for (QualifiedResource jc : listJC) {
	    ((JobControl)jc).killAll();
	    logger.debug("killAll() called on " + jc.getResource().getURL());
	}
    }
	
    /* */
    private int getCrateNumber(int fed) {
	int crate = 1;

	if (fed == 760) {
	    crate = 3; // TF-FED
	} else if (fed == 752 || (fed / 10) == 83) {
	    crate = 2;
	}

	return crate;
    }

    /* */
    private int getSlotNumber(int fed) {
	int slot = 0;

	switch (fed) {
	case 760: slot = 2; break; // TF

	case 750: slot = 8; break; // plus 1
	case 841: slot = 4; break;
	case 842: slot = 5; break;
	case 843: slot = 6; break;
	case 844: slot = 7; break;
	case 845: slot = 9; break;
	case 846: slot = 10; break;
	case 847: slot = 11; break;
	case 848: slot = 12; break;
	case 849: slot = 13; break;

	case 752: slot = 8; break; // plus 2
	case 831: slot = 4; break;
	case 832: slot = 5; break;
	case 833: slot = 6; break;
	case 834: slot = 7; break;
	case 835: slot = 9; break;
	case 836: slot = 10; break;
	case 837: slot = 11; break;
	case 838: slot = 12; break;
	case 839: slot = 13; break;
	}

	return slot;
    }

    private class TTSSetter implements Runnable {
	private int crate, slot, bits;
	private int repeat = 0;

	public synchronized void config(
					int crate, int slot, int bits, int repeat) {
	    this.crate = crate;
	    this.slot = slot;
	    this.bits = bits;
	    this.repeat = repeat * 16;

	    if (this.repeat == 0) {
		this.repeat = 1;
	    }
	}

	public void run() {
	    if (repeat > 0) {
		try {
		    logger.debug("TTSSetter: " +
				 crate + " " + slot + " " + bits + " " + repeat);

		    svTTSParameter.setValue("TTSCrate", "" + crate);
		    svTTSParameter.setValue("TTSSlot", "" + slot);
		    svTTSParameter.setValue("TTSBits", "" + bits);
		    svTTSParameter.send();
		    functionManager.gemSupervisor.execute(new Input("SetTTS"));

		    bits = (bits + 1) % 16;  // prepare for the next shot.
		    repeat--;

		    if (repeat == 0) {
			functionManager.fireEvent(GEMInputs.SETTTSTEST_MODE);
		    }
		} catch (Exception e) {
		    logger.error("TTSSetter", e);
		    functionManager.fireEvent(GEMInputs.SETERROR);
		}
	    }
	}
    }


}

// End of file
