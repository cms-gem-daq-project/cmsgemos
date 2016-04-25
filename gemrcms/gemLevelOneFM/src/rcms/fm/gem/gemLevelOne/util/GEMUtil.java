package rcms.fm.gem.gemLevelOne.util;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.math.BigInteger;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import com.sun.tools.doclets.internal.toolkit.util.Group;

import net.hep.cms.xdaqctl.XDAQMessageException;
import net.hep.cms.xdaqctl.XDAQTimeoutException;
import net.hep.cms.xdaqctl.XMASMessage;
import net.hep.cms.xdaqctl.xdata.FlashList;
import net.hep.cms.xdaqctl.xdata.SimpleItem;
import net.hep.cms.xdaqctl.xdata.TimeVal;
import net.hep.cms.xdaqctl.xdata.XDataType;
import rcms.fm.fw.parameter.CommandParameter;
import rcms.fm.fw.parameter.FunctionManagerParameter;
import rcms.fm.fw.parameter.Parameter;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.parameter.type.DateT;
import rcms.fm.fw.parameter.type.DoubleT;
import rcms.fm.fw.parameter.type.IntegerT;
import rcms.fm.fw.parameter.type.StringT;
import rcms.fm.fw.parameter.type.VectorT;
import rcms.fm.fw.parameter.util.ParameterFactory;
import rcms.fm.fw.parameter.util.ParameterUtil;
import rcms.fm.fw.service.parameter.ParameterServiceException;
import rcms.fm.gem.gemLevelOne.GEMFunctionManager;
import rcms.fm.gem.gemLevelOne.GEMParameters;
import rcms.fm.gem.gemLevelOne.GEMStates;
import rcms.fm.gem.gemLevelOne.gui.GuiStatePanel;
import rcms.fm.gem.gemLevelOne.gui.GuiSubdetPanel;
import rcms.fm.resource.QualifiedResource;
import rcms.fm.resource.qualifiedresource.FunctionManager;
import rcms.fm.resource.qualifiedresource.XdaqExecutive;
import rcms.fm.resource.qualifiedresource.JobControl;
import rcms.fm.resource.qualifiedresource.XdaqExecutiveConfiguration;
import rcms.resourceservice.db.resource.config.ConfigProperty;
import rcms.resourceservice.db.resource.fm.FunctionManagerResource;
import rcms.util.logger.RCMSLogger;
import rcms.utilities.elogPublisher.ElogPublisher;
import rcms.utilities.hwcfg.HWCFGInterface;
import rcms.utilities.runinfo.RunInfo;
import rcms.utilities.runinfo.RunInfoException;

public class GEMUtil {
    
    /**
     * <code>logger</code>: log4j logger.
     */
    static RCMSLogger logger = new RCMSLogger(GEMUtil.class);

    private GEMFunctionManager functionManager = null;

    private final BlockingQueue<Vector<Object>> publisherQueue = new ArrayBlockingQueue<Vector<Object>>(2000);

    private Thread _pubThread = null;

    private Runnable _publisher = null;

    public GEMUtil(GEMFunctionManager functionManager) {
	super();
	this.functionManager = functionManager;

	// start publisher thread
	//_publisher = new PublishConsumer(publisherQueue);
	//_pubThread  = new Thread(_publisher);
	//_pubThread.start();

    }

    /**
     * Hide default constructor
     */
    protected GEMUtil() {
	super();
    }

    /* (non-Javadoc)
     * @see java.lang.Object#finalize()
     */
    @Override
	protected void finalize() throws Throwable {
	super.finalize();
	destroy();
    }

    public void prepareGlobalConfigurationKey() {

    }

    public void prepareParametersForInitialized() throws Exception {
	// check for HLT key
    }

    /*
      public void updateRunKeys() {

      }
	
      private boolean checkFedConsistency() {}

      public boolean ttsStatusActive() {}
    */
    // check status
    // convention is 
    // bit mask for fed status is
    // bit     1  /  0
    // 0 SLINK ON / OFF
    // 1 TTC   ON / OFF
    // 2 & 0 SLINK NA / BROKEN
    // 3 & 1 TTC   NA / BROKEN 
    // 4 NO CONTROL
    // 5 FMM status bit 0 
    // 6 FMM status bit 1
    // 7 FMM status bit 2
    // 8 FMM status bit 3
    // 9 SLINK backpressure status OFF
    //10 SLINK backpressure status ON
	

    /**
     * Put a shutdown event on the queue to close the publisher thread.
     */
    public void destroy() throws InterruptedException {
	    
	Vector v = new Vector();
	//v.add(new ShutDownPublisher());
	//publisherQueue.add(v);
    }


    static public String getXmlRscConf(String xmlRscConf, String elementName) {

	// response string
	String response = "";

	// check the _xmlRscConf and _documentConf are filled
	if (xmlRscConf == null || xmlRscConf.equals("") ) 
	    return response;

	// check for a valid argument
	if (elementName == null || elementName.equals("") ) 
	    return response;
	int beginIndex = xmlRscConf.indexOf("<"+elementName+">") + elementName.length() + 2;
	int endIndex   = xmlRscConf.indexOf("</"+elementName+">");
	if (beginIndex<0 || endIndex<0) return response;
	response = xmlRscConf.substring(beginIndex, endIndex);

	return response;
    }

    public String myGetRscConfElement(String elementName) {

	// get the FM's resource configuration
	String elementValue = null;

	// try first properties field
	List<ConfigProperty> l = functionManager.getGroup().getThisResource().getProperties();

	if (l!=null) {
	    for (ConfigProperty p : l) {
		if (p.getName().equals(elementName)) {
		    elementValue = p.getValue();
		    break;
		}
	    }
	}

	return elementValue;

    }

	
    /**
     * Kill orphaned XDAQ executives
     */
    public void killOrphanedExecutives() {
	String pathToOrphanHandler = myGetRscConfElement( "pathToOrphanHandler" );
	if ( pathToOrphanHandler == null ){
	    logger.warn("No orphan handler script found in the configuration of this Function Manager.");
	    return;
	}
	if ( pathToOrphanHandler == "" ){
	    logger.warn("No orphan handler script found in the configuration of this Function Manager.");
	    return;
	}

	System.out.println("Orphan handler script '"+pathToOrphanHandler+"' will be used.");
	logger.debug("Orphan handler script '"+pathToOrphanHandler+"' will be used.");
	    
	List<QualifiedResource> jobcontrols = functionManager.getQualifiedGroup().seekQualifiedResourcesOfType(new JobControl());
	for (QualifiedResource jc: jobcontrols) {
	    try{
		List<String> orphanHandlingJIDs = new ArrayList<String>();
		List<QualifiedResource> executives = functionManager.getQualifiedGroup().seekQualifiedResourcesOfType(new XdaqExecutive());
		for (QualifiedResource exe: executives){
		    // We assume a single jobcontrol on each host and compare host names only:
		    if ( jc.getURL().getHost().equals( exe.getURL().getHost() ) ){
			// This doesn't seem to work: String user = ((XdaqExecutive)exe).getJobUserName();
			// Get the user name indirectly::
			XdaqExecutiveConfiguration conf = ((XdaqExecutive)exe).getXdaqExecutiveConfiguration();
			if ( conf != null ){
			    String user = conf.getUserName();
			    String args = Integer.toString( exe.getURL().getPort() );
			    String environment = "USER=" + user;
			    String jid = pathToOrphanHandler + "_" + Integer.toString( exe.getURL().getPort() );
			    ((JobControl)jc).start(pathToOrphanHandler,args,environment,user,jid);
			    logger.debug("Issued to JobControl " + jc.getURL() + " the command: " + environment + " " + pathToOrphanHandler + " " + args + ", jid=" + jid );
			    orphanHandlingJIDs.add( jid );
			}
			else{
			    logger.error( "XdaqExecutiveConfiguration is null for " + exe.toString() ); 
			}
		    }
		}
		// Allow the script some time to do its dirty job:
		wait( 1000 );
		// Clean up orphan-handling job entries
		for ( String jid : orphanHandlingJIDs ){
		    logger.debug("Killing orphan handling job " + jid + " of JobControl " + jc.getURL() );
		    ((JobControl)jc).killJid( jid );
		}
	    } catch (Exception e) {
		logger.error( "Failed to execute orphan handler on JobControl " + jc.getURL(), e);
	    }
	}
    }

    /**
     * Set HWCFG_KEY and HWCFG_TRG_NAME parameters
     */
    public void prepareHwcfgKey() {
	// get hwcfg element
	/*
	  String hwcfgKey = myGetRscConfElement( "hwcfgKey" );
	  if (hwcfgKey.equals("")) {
	  // set to default to limp on...
	  hwcfgKey="hwcfg_cessy_default";
	  }
	  // debug
	  logger.info("prepareHwcfgKey() hwcfgKey = " + hwcfgKey);
	  // put it into the corresponding parameter
	  functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.HWCFG_KEY,new StringT(hwcfgKey)) );

	  // overwrite with HWCFG_KEY from DAQ
	  //setHwcfgKeyFromDaq();

	  // HWCFG_TRG_NAME parameter ------------------
	  // this is used with the hwcfg DB to identify the trigger
	  // and find ttc partitions. not used for role name and alike.
		
	  // debug
	  logger.info("prepareHwcfgKey() from DAQ hwcfgKey = " + functionManager.getParameterSet().get(GEMParameters.HWCFG_KEY).getValue().toString());

	  // get hwcfg element
	  String hwcfgGEMName = myGetRscConfElement( "hwcfgGEMName" );

	  if (hwcfgGEMName.equals("")) {
	  // set to default to limp on...
	  hwcfgGEMName="GEMP";
	  }

	  // debug
	  logger.info("prepareHwcfgKey() hwcfgGEMName = " + hwcfgGEMName );

	  // put it into the corresponding parameter
	  functionManager.getParameterSet().put(new FunctionManagerParameter<StringT>(GEMParameters.HWCFG_GEM_NAME, new StringT(hwcfgGEMName)) );
	*/	
    }

    /**
     * @param ms
     *       wait before resuming the thread.
     */
    static public void wait(int ms) {
	    
	try {
	    Thread.sleep(ms);
	} catch (InterruptedException e) {
	    e.printStackTrace();
	}
	    
    }

    public void setParameterFromProperties() {
		
	List<ConfigProperty> lp = functionManager.getGroup().getThisResource().getProperties();
	logger.info("start properties");
	for (ConfigProperty p : lp) {
	    logger.info("geting properties" + p.getName());
	    if (p.getName().equals("Calibration_Keys")) {
		logger.info("ends properties");
		functionManager.getParameterSet().get(GEMParameters.GEM_CALIB_KEYS_AVAILABLE)
		    .setValue(new StringT( p.getValue() ));
	    }
	}
    }
	
    public void renderMainGui() {

	GuiStatePanel guiState = new GuiStatePanel(functionManager);
	functionManager
	    .getParameterSet()
	    .get(GEMParameters.GUI_STATE_PANEL_HTML)
	    .setValue(new StringT( guiState.generateHtml() ));

	GuiSubdetPanel guiSubdet = new GuiSubdetPanel(functionManager);
	functionManager
	    .getParameterSet()
	    .get(GEMParameters.GUI_SUBDET_PANEL_HTML)
	    .setValue(new StringT( guiSubdet.generateHtml() ));

    }

}

