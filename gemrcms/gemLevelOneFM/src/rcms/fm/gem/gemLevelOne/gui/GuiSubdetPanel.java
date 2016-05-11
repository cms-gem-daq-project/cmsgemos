package rcms.fm.gem.gemLevelOne.gui;

import org.apache.log4j.Logger;

import com.sun.corba.se.impl.copyobject.JavaStreamObjectCopierImpl;

import rcms.fm.gem.gemLevelOne.GEMFunctionManager;
import rcms.fm.gem.gemLevelOne.GEMParameters;

public class GuiSubdetPanel {
    private static final long serialVersionUID = 1L;
    
    private static Logger logger = Logger.getLogger(GuiSubdetPanel.class);
    
    private GEMFunctionManager fm;
    
    public GuiSubdetPanel( GEMFunctionManager fm ) {
	super();
	this.fm = fm;
    }
    
    public String generateHtml() {
	String html = "";
	
	// create a table with all subsystems
	html += "<!-- Table subdet panel begin -->";
	html += "<table cellpadding=\"5pt\" cellspacing=\"4pt\" >";
	html += "<tbody>\n";
	
	// row run keys			
	html += "<tr>\n";
	html += "\n<td class=\"label\">Calibration_Keys</td>";
	html += "\n<td style=\"font-size: 1em; text-align: center; \">"; 
	html += "\n<select class=\"RunDataInputTable\" name=\"GEM_CALIB_KEYS\" onChange=\"setRunDataParameterSelect(\'GEM_CALIB_KEY\','"
	    + "rcms.fm.fw.parameter.type.StringT' ,this)\">";
	String calibKeys = fm.getParameterSet().get( GEMParameters.GEM_CALIB_KEYS_AVAILABLE).getValue().toString();
		
	// make a drop down
	if(calibKeys.length()>2) {
	    // rm []
	    calibKeys = calibKeys.substring(1, calibKeys.length()-1);
	    // split by ,
	    String[] keys = calibKeys.split(",");
	    html += "\n<option id=\"GEM_CALIB_KEYS\">";
	    //html += fm.getParameterSet().get("GEM_CALIB_KEY").getValue().toString();
	    for (String key : keys) {
		// rm "
		key = key.substring(key.indexOf("\"")+1);
		key = key.substring(0,key.indexOf("\""));
		html += "\n<option value=\"" + key + "\">" + key;
	    }	

	}
	html += "\n</select> </td>";

	html += "</tr>\n";

	html += "</tbody>\n";	
	html += "</table>\n";	
	return html;

    } 

    private boolean checkRenderCondition(String subsys) {
	/*
	  if ( subsys != null 
	  && fm.getParameterSet().get(subsys) != null 
	  && !fm.getParameterSet().get(subsys).getValue().toString().equals(GEMParameters.AWAY) 
	  && !fm.getParameterSet().get(subsys).getValue().toString().equals(GEMParameters.OUT) 
	  ) { 
	  return true;
	  }
	  else {
	  return false;
	  }
	*/
	return false;
    }
}
