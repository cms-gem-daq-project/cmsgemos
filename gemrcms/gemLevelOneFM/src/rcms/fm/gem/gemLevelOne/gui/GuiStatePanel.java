package rcms.fm.gem.gemLevelOne.gui;

import rcms.fm.gem.gemLevelOne.GEMFunctionManager;

public class GuiStatePanel {
    
    private GEMFunctionManager fm;
    
    public GuiStatePanel( GEMFunctionManager fm ) {
	super();
	this.fm = fm;
    }
    
    public String generateHtml() {
	
	String state = "Initial";
	if (fm!=null&&fm.getState()!=null) state = fm.getState().getStateString();			
	String html = "";
	html += "<div style=\"height: 40px; padding-top: 5px; padding-left: 15px;\" class=\""+ state + "\" id='currentState'>";
	html += state + "</div>";
	return html;
	
    }
}


