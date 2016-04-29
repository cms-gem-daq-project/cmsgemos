package rcms.fm.gem.gemLevelOne;

import rcms.fm.fw.EventHandlerException;
import rcms.fm.fw.parameter.FunctionManagerParameter;
import rcms.fm.fw.parameter.Parameter;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.parameter.type.DoubleT;
import rcms.fm.fw.parameter.type.IntegerT;
import rcms.fm.fw.parameter.type.StringT;
import rcms.fm.fw.service.parameter.ParameterServiceException;
import rcms.fm.fw.user.UserActionException;
import rcms.fm.fw.user.UserEventHandler;
import rcms.util.logger.RCMSLogger;

/**
 * Defined Level 1 Function Manager parameters.
 * 
 * Standard parameter definitions for Level 1 Function Manager
 * 
 * SID					: Session Identifier
 * STATE				: State name the function manager is currently in
 * RUN_TYPE				: String identifying the global run type 
 * RUN_MODE				: String identifying the global run mode
 * GLOBAL_CONF_KEY		 	: String representing the global configuration key
 * RUN_NUMBER				: Run number of current run
 * ACTION_MSG 				: Short description of current activity, if any
 * ERROR_MSG 				: In case of an error contains a description of the error
 * COMPLETION 				: Completion of an activity can be signaled through this numerical value 0 < PROGRESS_BAR < 1
 * 
 * For more details => https://twiki.cern.ch/twiki/bin/view/CMS/StdFMParameters
 * 
 * @author Andrea Petrucci, Alexander Oh, Michele Gulmini
 *
 */
public class GEMSetParameterHandler extends UserEventHandler {

    static RCMSLogger logger = new RCMSLogger(GEMSetParameterHandler.class);

    private GEMFunctionManager functionManager = null;

    public GEMSetParameterHandler() throws EventHandlerException {

        subscribeForEvents(ParameterSet.class);

        // Adds callbacks action associated to a specific Function Manager
        // State.
        addAnyStateAction("onParameterSet");
    }

    /* (non-Javadoc)
     * @see rcms.fm.fw.user.UserActions#init()
     */
    public void init() throws EventHandlerException {
        functionManager = (GEMFunctionManager) getUserFunctionManager();
    }

    /* callback */
    public void onParameterSet(ParameterSet parameters)
	throws UserActionException {

        // debug
        logger.info("paramters: " + parameters);
        String debug="";
        for (Object obj : parameters.getParameters() ) {
            FunctionManagerParameter fp = (FunctionManagerParameter)obj;
            debug += fp.getName() + ":" + fp.getValue() + "\n";
        }
        for (Object obj : parameters.getNames() ) {
            String s = (String)obj;
            debug += s + "\n";
        }
        logger.info("onParameterSetSet parameter of GEM FM is called.\n" + debug);

        processGuiCommands(parameters);

        logger.info("parameters: " + parameters);
    }



    private void processGuiCommands(ParameterSet parameters) {
        /*
        // copy gui command
        Parameter p = parameters.get(GEMParameters.GUI_COMMAND);
        if (p==null) {
        logger.debug("no gui command found.");
        return;
        }

        String guiCommand = p.getValue().toString();

        // reset gui command so that it will be not executed twice in case
        // the parameter set is send again (this is the case when we
        // distribute the parameters to all levelOne FMs).
        functionManager.getParameterSet().put( new FunctionManagerParameter<StringT>(GEMParameters.GUI_COMMAND, new StringT("")) );


        if (guiCommand != null && guiCommand.length()>0 ) {

        if (guiCommand.equals("restoreSetup")) { }
        }
        */
    }

}
