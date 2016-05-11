package rcms.fm.gem.gemLevelOne;

import java.util.List;
import java.util.ArrayList;

import rcms.fm.gem.gemLevelOne.util.GEMUtil;
import rcms.fm.fw.parameter.CommandParameter;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.user.UserActionException;
import rcms.fm.fw.user.UserFunctionManager;
import rcms.fm.resource.QualifiedGroup;
import rcms.fm.resource.QualifiedResource;
import rcms.fm.resource.QualifiedResourceContainer;
import rcms.fm.resource.qualifiedresource.FunctionManager;
import rcms.fm.resource.qualifiedresource.XdaqApplicationContainer;
import rcms.fm.resource.qualifiedresource.XdaqExecutive;
import rcms.statemachine.definition.State;
import rcms.statemachine.definition.StateMachineDefinitionException;
import rcms.util.logger.RCMSLogger;
import rcms.utilities.runinfo.RunInfo;


/**
 * Function Machine for controlling the GEM Level 1 Function Manager.
 * 
 * @author Andrea Petrucci, Alexander Oh, Michele Gulmini
 * modified for use in the GEM system by Jared Sturdy
 *
 */
public class GEMFunctionManager extends UserFunctionManager {

    /**
     * <code>RCMSLogger</code>: RCMS log4j Logger
     */
    static RCMSLogger logger = new RCMSLogger(GEMFunctionManager.class);

    /**
     * define the xdaq application containers
     */
    public XdaqApplicationContainer containerXdaqApplication = null;

    public XdaqApplicationContainer cEVM = null;
    public XdaqApplicationContainer containerXdaqExecutive = null;
    public XdaqApplicationContainer containergemSupervisor = null;
    public XdaqApplicationContainer containerTTCciControl  = null;
    public XdaqApplicationContainer containerTCDSControl   = null;
    
    /**
     * copied from HCAL, possibly able to incorporate them for use
     * in the GEM system
     */
    public XdaqApplicationContainer containerEVM                 = null;
    public XdaqApplicationContainer containerBU                  = null;
    public XdaqApplicationContainer containerRU                  = null;
    public XdaqApplicationContainer containerFUResourceBroker    = null;
    public XdaqApplicationContainer containerFUEventProcessor    = null;
    public XdaqApplicationContainer containerStorageManager      = null;
    public XdaqApplicationContainer containerFEDStreamer         = null;
    public XdaqApplicationContainer containerPeerTransportATCP   = null;
    public XdaqApplicationContainer containergemRunInfoServer    = null;
    

    /**
     * <code>containerFunctionManager</code>: container of FunctionManagers
     * in the running Group.
     */
    public QualifiedResourceContainer containerFunctionManager = null;
    public QualifiedResourceContainer containerJobControl      = null;

    /**
     * <code>calcState</code>: Calculated State.
     */
    public State calcState = null;

    // RunInfo stuff:
    public RunInfo _gemRunInfo        = null;
    public RunInfo _gemRunInfoDESTROY = null;

    // utlitity functions handle
    public GEMUtil _gemUtil;



    // The GEM FED ranges
    protected Boolean GEMin = false;
    protected final Integer firstGEMFedId = 1000;
    protected final Integer lastGEMFedId  = 1010;


    /**
     * Instantiates an GEMFunctionManager.
     */
    public GEMFunctionManager() {
	// Any State Machine Implementation must provide the framework
	// with some information about itself.

	// make the parameters available
	addParameters();

    }

    /*
     * (non-Javadoc)
     * 
     * @see rcms.statemachine.user.UserStateMachine#createAction()
     * This method is called by the framework when the Function Manager is created.
     */
    public void createAction(ParameterSet<CommandParameter> pars) throws UserActionException {
	String message = "[GEM] gemLevelOneFM createAction called.";
	System.out.println(message);
	logger.debug(      message);

	_gemUtil.killOrphanedExecutives();

	message = "[GEM] gemLevelOneFM createAction executed.";
	System.out.println(message);
	logger.debug(      message);
    }
    
    /*
     * (non-Javadoc)
     * @see rcms.statemachine.user.UserStateMachine#destroyAction()
     * This method is called by the framework when the Function Manager is destroyed.
     */
    public void destroyAction() throws UserActionException {
	String message = "[GEM] gemLevelOneFM destroyAction called.";

	System.out.println(message);
	logger.debug(      message);

	logger.debug("[GEM] gemLevelOneFM destroyAction");

	QualifiedGroup group = getQualifiedGroup();

	List<QualifiedResource> list;

	// destroy XDAQ executives
	list = group.seekQualifiedResourcesOfType(new XdaqExecutive());

	for (QualifiedResource r: list) {
	    logger.debug("==== killing " + r.getURI());
	    try {
		((XdaqExecutive)r).killMe();
	    } catch (Exception e) {
		logger.error("Could not destroy a XDAQ executive " + r.getURI(), e);
	    }
	}

	// destroy function managers
	list = group.seekQualifiedResourcesOfType(new FunctionManager());

	for (QualifiedResource r: list) {
	    logger.debug("==== killing " + r.getURI());
	    
	    FunctionManager fm = (FunctionManager)r;
	    
	    if (fm.isInitialized()) {
		try {
		    fm.destroy();
		} catch (Exception e) {
		    logger.error("Could not destroy a FM " + r.getURI(), e);
		}
	    }
	}
	
	message = "[GEM] gemLevelOneFM destroyAction executed";
	System.out.println(message);
	logger.debug(      message);
    }

    /**
     * add parameters to parameterSet. After this they are accessible.
     */
    private void addParameters() {
	parameterSet = GEMParameters.LVL_ONE_PARAMETER_SET;
    }

    public void init() throws StateMachineDefinitionException,
			      rcms.fm.fw.EventHandlerException {

	//instantiate utility
	_gemUtil = new GEMUtil(this);

	// Set first of all the State Machine Definition
	setStateMachineDefinition(new GEMStateMachineDefinition());

	// Add event handler
	addEventHandler(new GEMEventHandler());

	// add SetParameterHandler
	addEventHandler(new GEMSetParameterHandler());
		
	// Add error handler
	addEventHandler(new GEMErrorHandler());

	// get session ID
	getSessionId();

	// call renderers
	_gemUtil.renderMainGui();
    }

    /**
     * Returns true if custom GUI is required, false otherwise
     * @return true, because GEMFunctionManager class requires user code
     */	
    public boolean hasCustomGUI() {
	return true;
    }

}
