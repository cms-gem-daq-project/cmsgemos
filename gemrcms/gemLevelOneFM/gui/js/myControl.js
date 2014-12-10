/* Onload function to initialize stuff*/
// globals
var myAnim=null;
var _blackboardSelected;

function setRunDataParameterSelect(formName,type,selectElement)
{
    // set the new value in the form element and update the
    // FM.
	document.FMPilotForm.globalParameterName3.value=formName;
	document.FMPilotForm.globalParameterValue3.value=selectElement.options[selectElement.selectedIndex].value;
	document.FMPilotForm.globalParameterType3.value=type;
	
	submitForm('FMPilotForm', 'SET_GLOBAL_PARAMETERS');   
	
	// reset selection to avoid that with the next POST the same command is send
	document.FMPilotForm.globalParameterName3.value="x";
	document.FMPilotForm.globalParameterValue3.value="x";
	document.FMPilotForm.globalParameterType3.value="x";
}
 
// takes a value
function setMyParameterValue(formName,type,value)
{
    // set the new value in the form element and update the
    // FM.
	document.FMPilotForm.globalParameterName3.value=formName;
	document.FMPilotForm.globalParameterValue3.value=value;
	document.FMPilotForm.globalParameterType3.value=type;
	
	submitForm('FMPilotForm', 'SET_GLOBAL_PARAMETERS');   
}
 // takes an element
function setMyParameter(formName,type,textElement)
{
    // set the new value in the form element and update the
    // FM.
	document.FMPilotForm.globalParameterName4.value=formName;
	document.FMPilotForm.globalParameterValue4.value=textElement.value;
	document.FMPilotForm.globalParameterType4.value=type;
	
	submitForm('FMPilotForm', 'SET_GLOBAL_PARAMETERS');   
}

 
function setRunDataParameterInput(formName,type,inputElement)
{
    // check whether you are allowed to modify
	if (isModifiable()==false) {
	  // force reload to keep correct states
	  //onUpdatedRefreshButton();
	  return;
	}
	
    // set the new value in the form element and update the
    // FM.
	document.FMPilotForm.globalParameterName3.value=formName;
	document.FMPilotForm.globalParameterValue3.value=inputElement.value;
	document.FMPilotForm.globalParameterType3.value=type;
	submitForm('FMPilotForm', 'SET_GLOBAL_PARAMETERS');   
}
 /*----------------------------------------------------------------------------*/
 
/* Draws the state label value */
function drawState(state) {
	
	if (state != null) {
	
		var htmlObject = document.getElementById('currentState');
		
		if (htmlObject != null) {
			if (htmlObject.innerHTML !=  state.value) {
				 var onloadSound = document.getElementById(state.value+'Sound');
				 if ( onloadSound != null ) playSound(  onloadSound.value );
			}
			// update value
			htmlObject.innerHTML = state.value;
			htmlObject.className  = state.value;
		}
				
	}
}



/* update global parameters */
function myUpdateParameters(message) {

	var pArray = message.getElementsByTagName('PARAMETER');

	var pDebug = document.getElementById('DEBUG_MSG');

    var debugString = '';

	var pObject = null;
	if (pArray != null) {
			
		for (i = 0; i < pArray.length; i++) {
				
			if (pArray[i] != null) {
				pNameNode = pArray[i].getElementsByTagName('NAME')[0];
				pValueNode = pArray[i].getElementsByTagName('VALUE')[0];
				if (pNameNode != null && pValueNode != null) {
					pName = pNameNode.childNodes[0].nodeValue;
					
					if ( pValueNode.childNodes[0] != null) {
						pValue = pValueNode.childNodes[0].nodeValue;
					
						// DEBUG
						if (pDebug != null) {
   						  debugString = debugString + "name=" + pName + " value=" + pValue + "\n";
						  pDebug.innerHTML = debugString;
						}
					
						// special case for fed status
						else if (pName.indexOf("_FEDS")  != -1)
						{
							processFedStatus( pName, pValue ); 
						}
					
						var pObject = document.getElementById(pName);
						// identification can be only by id...
						// arbitrary atributes don't work in mozilla...
						//
						if (pObject != null) {
							// special case for SUBDET/DCS state
							// set also the class
							
							if (pName == "HEARTBEAT"){
								doAnim();
							} 
							
							else if (pName == "currentState") { 
								pObject.className = pValue; 
								pObject.innerHTML = pValue;
							}
							else if (pName.indexOf("_STATE")  != -1) { 
								pObject.className = pValue; 
								pObject.innerHTML = pValue;
							}
							else if (pName == "TTS_TEST_SUMMARY" && pValue != "") {
								processTtsTestSummary(pValue);
							}
							else {
								pObject.innerHTML = pValue;
							}
							
						}
					}
				}
			}
		}
	}
}


		


function myInit()
{
	_blackboardSelected=false;
	 onLoad(); 
	 var onloadSound = document.getElementById('onloadSound'); 
	 if (onloadSound != null) {
		playSound(onloadSound.value);
	 }
}

// create the progress bar
function initProgressBar() {
    var centerCellName;
    var tableText = "";
    var idiv = document.getElementById("DAQ_PROGRESS");
}

// show the current percentage
function setProgressBarValue(percentage,subsys) {
    var percentageText = "";
    if (percentage < 10) {
        percentageText = "&nbsp;" + percentage;
    } else {
        percentageText = percentage;
    }

    var idiv = document.getElementById(subsys);	
    if (idiv!=null) { 
        idiv.innerHTML = "<font color=\"red\" size=1px>" + percentageText + "%</font>";
    }
}

// process fed status
function processFedStatus( formName, fedString ) {	

	if(fedString==null) return;
	
    // parse fedString
	arrayOfFeds=fedString.split("%");
	
	if(arrayOfFeds==null) return;
	
	for (var i=0; i < arrayOfFeds.length; i++) {
	 
	  if (arrayOfFeds[i]==null) return;
	  
	  // split into id,status
	  s = arrayOfFeds[i].split("&");
	
	  if (s==null) return;
	  if (s.length!=2) return;
	  
	  
	  id = s[0];
	  status = s[1];
	  
	  if(id==null || status==null) return;
	  
	  // check for empty and space strings
	  if (id=="" || id==" ") return;	  
	  if (status=="" || status==" ") return;
	  
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
	  
	  var bit0 =                   2*2*2*2*2;
	  var bit1 =                 2*2*2*2*2*2;
	  var bit2 =               2*2*2*2*2*2*2;
	  var bit3 =             2*2*2*2*2*2*2*2;
	  var bitSlinkNoBp =   2*2*2*2*2*2*2*2*2;
	  var bitSlinkBp   = 2*2*2*2*2*2*2*2*2*2;
	  
	  var ready 		  = bit3;
	  var busy  		  = bit2;
	  var sync  		  = bit1;
	  var warn  		  = bit0;
	  var err   		  = bit2 + bit3;
	  var disconnected = bit0 + bit1 + bit2 + bit3;
	  var ttsBitmask   = bit0 + bit1 + bit2 + bit3;
	  
	  var elementIdSlink = 'slink'+formName.substr(0,(formName.length-5))+id;
	  
	  // SLINK must be in control
	  if ( (status & 1)!=0 && (status & 16)==0) {
	 
	 	  // fetch html element
	  	  var buttonSlink = document.getElementById(elementIdSlink);
	  	
	  	  if (buttonSlink!=null) {
		  	  // get status
			  if ((status & bitSlinkBp)!=0)		{
			  	buttonSlink.className='slinkBp';	
			  }  
			  else if ((status & bitSlinkNoBp)!=0)	{
			  	buttonSlink.className='slinkNoBp';	
			  }   
		  }
	  
	  }
	  
	  var elementId = 'tts'+formName.substr(0,(formName.length-5))+id;
	  
	  // TTC must be on and in control
	  if ( (status & 2)!=0 && (status & 16)==0) {
	  
	      // fetch html element
	  	  var button = document.getElementById(elementId);
	  	
	  	  if (button!=null) {
		  	  // get status
			  if ((status & ttsBitmask)==ready)		{
			  	button.className='ttsReady';	
			  }  
			  else if ((status & ttsBitmask)==busy)	{
			  	button.className='ttsBusy';	
			  }   
			  else if ((status & ttsBitmask)==sync)	{
			  	button.className='ttsSync';
			  }   
			  else if ((status & ttsBitmask)==warn)	{
			  	button.className='ttsWarn';
			  }   
			  else if ((status & ttsBitmask)==err)	{
			  	button.className='ttsErr';
			  }   
			  else if ((status & ttsBitmask)==disconnected)	{
			  	button.className='ttsDisconnected';
			  }   
			  
		  }
	  }
   }
}

/* Draws the command button section */
function drawMyCommandButtons(state) {
	if (commands != null && commands.length != 0) {
		for (var index = 0; index < commands.length; index++) {
			if (isCommandEnabled(transitions, commands[index], state)) {
				// enable button
				var button = document.getElementById(commands[index].input+'Button');
				if (button != null) {
					button.disabled = false; 
					button.style.color = "yellow";
				}
			}
			else {
			  	//disable button
				var button = document.getElementById(commands[index].input+'Button');
				if (button != null) {
					button.disabled = true;
					button.style.color = "grey";
					
				} 
			  	
			  }
		}
	}
	
}


function nothing() {
 	// do nothing
 	return;
}


/* Play sound URL's */
function playSound(surl) {
  var sound = document.getElementById('soundspan');
  if (sound == null) return;
  sound.innerHTML=
    "<embed src='"+surl+"' hidden=true autostart=true loop=false>";
}

/* check if state running -> ask for confirmation, else destroy */

function onMyDestroyButton() {
    // check my state
    var htmlObject = document.getElementById('currentState');
		
    if (htmlObject != null) {
	
		if (
			htmlObject.innerHTML == 'Starting' || 
			htmlObject.innerHTML == 'Running' || 
			htmlObject.innerHTML == 'Paused'  || 
			htmlObject.innerHTML == 'Pausing' || 
			htmlObject.innerHTML == 'Resuming' 			
		) {
	  
			// get confirmation by user
			var destroySound = document.getElementById('destroySound');
			if ( destroySound != null ) playSound( destroySound.value );

			if (confirm("Destroying in this State does not close the current Run properly. \nAre you sure you want to destroy now?") == false) {
			  return;
			}
				
		}
		// get confirmation by user
		if (confirm("Everything will be terminated!\nAre you sure you want to destroy?") == false) {
		  return;
		}
	}	
	disableAllButtons(buttons);
	closeStateTreeWindow();
	submitForm('FMPilotForm', 'Destroy');
}

		
function openKeySelector() {
    var htmlObject = document.getElementById('treeJsp');
	var groupID = document.getElementById('groupID');
	window.open(htmlObject.value+groupID.value,'HLT Key Selector','scrollbars=yes,height=600,width=800,resizable=yes,toolbar=no,location=no,status=no,menubar=no');
    submitForm('FMPilotForm', 'UpdatedRefresh');
}

function openFedTtsConfig() {
    var htmlObject = document.getElementById('fedTtsConfigJsp');
	var groupID = document.getElementById('groupID');
	window.open(htmlObject.value+groupID.value,'FED TTS Configurator','scrollbars=yes,height=600,width=800,resizable=yes,toolbar=no,location=no,status=no,menubar=no');
    submitForm('FMPilotForm', 'UpdatedRefresh');
}

function openLink(link) {
	window.open(link,link,'scrollbars=yes,height=600,width=800,resizable=yes,toolbar=no,location=no,status=no,menubar=no');
}
function processTtsTestSummary( ttsTestSummary ) {	

	if(ttsTestSummary==null) return;
	
    // parse fedString
	arrayOfTts=ttsTestSummary.split("&");
	
	if(arrayOfTts==null) return;
	
	for (var i=0; i < arrayOfTts.length; i++) {
	 
	  if (arrayOfTts[i]==null) return;
	  
	  // split into id,status
	  s = arrayOfTts[i].split("%");
	
	  if (s==null) return;
	  
	  if (s.length==4) {
		  
		  id = s[0];
		  status = s[1];
		  seq = s[2];
		  comment = s[3];
		  
		  if(id==null || status==null || seq==null || comment==null) return;
		  
		  // check for empty and space strings
		  if (id=="" || id==" ") return;	  
		  if (status=="" || status==" ") return;
		  
		 
		  var elementTts=id;
		  var elementSeq="s"+id;
		  var elementComment="c"+id;
		  
		  // fetch html element
		  var tdTts = document.getElementById(elementTts);
		  tdTts.className = 'testresult' + status;
		  
		  var tdTtsContent = document.getElementById(elementSeq);
		  if (seq!="") { 
		  	tdTtsContent.innerHTML = seq;
		  }
		  
		  var tdTtsComment = document.getElementById(elementComment);
		  if (comment!="") { 
		  	tdTtsComment.innerHTML = comment;
		  }
		  
	  }
   }
}

function onClickMyCommandButton(commandName,elementid) {
	var commandObj = document.getElementById('COMMAND');
	commandObj.value = commandName;
    disableAllButtons(buttons);
 //	closeXMLHttpConnection();
	var form = document.getElementById('FMPilotForm');
	form.ACTION.value = 'Execute';
	AjaxRequest.submit( form );
//	submitForm('FMPilotForm', 'Execute');
    enableAllButtons(buttons);
}



function setAjaxRunDataParameterSelect(formName,type,selectElement)
{
    // set the new value in the form element and update the
    // FM.
	document.FMPilotForm.globalParameterName3.value=formName;
	document.FMPilotForm.globalParameterValue3.value=selectElement.options[selectElement.selectedIndex].value;
	document.FMPilotForm.globalParameterType3.value=type;
	
	var form = document.getElementById('FMPilotForm');
	form.ACTION.value = 'SET_GLOBAL_PARAMETERS';
	AjaxRequest.submit( form );
	
	// reset selection to avoid that with the next POST the same command is send
	document.FMPilotForm.globalParameterName3.value="x";
	document.FMPilotForm.globalParameterValue3.value="x";
	document.FMPilotForm.globalParameterType3.value="x";
}

function enableAllButtons(buttons) {
	if (buttons != null) {
		for(i = 0; i < buttons.length; i++) {
			enableField(buttons[i]);
		}
	}
}


function initAnim() {
	myAnim = new YAHOO.util.ColorAnim('HEARTBEAT', {backgroundColor: { from: '#00FF00' , to: '#008000' } }); 
}
 
function doAnim() { 
  if (myAnim==null) initAnim();
  myAnim.animate();
}

function processTtsTestSummary( ttsTestSummary ) {	

	if(ttsTestSummary==null) return;
	
    // parse fedString
	arrayOfTts=ttsTestSummary.split("&");
	
	if(arrayOfTts==null) return;
	
	for (var i=0; i < arrayOfTts.length; i++) {
	 
	  if (arrayOfTts[i]==null) return;
	  
	  // split into id,status
	  s = arrayOfTts[i].split("%");
	
	  if (s==null) return;
	  
	  if (s.length==4) {
		  
		  id = s[0];
		  status = s[1];
		  seq = s[2];
		  comment = s[3];
		  
		  if(id==null || status==null || seq==null || comment==null) return;
		  
		  // check for empty and space strings
		  if (id=="" || id==" ") return;	  
		  if (status=="" || status==" ") return;
		  
		 
		  var elementTts=id;
		  var elementSeq="s"+id;
		  var elementComment="c"+id;
		  
		  // fetch html element
		  var tdTts = document.getElementById(elementTts);
		  tdTts.className = 'testresult' + status;
		  
		  var tdTtsContent = document.getElementById(elementSeq);
		  if (seq!="") { 
		  	tdTtsContent.innerHTML = seq;
		  }
		  
		  var tdTtsComment = document.getElementById(elementComment);
		  if (comment!="") { 
		  	tdTtsComment.innerHTML = comment;
		  }
		  
	  }
   }
}

// BROWSER DETECTION
var BrowserDetect = {
	init: function () {
		this.browser = this.searchString(this.dataBrowser) || "An unknown browser";
		this.version = this.searchVersion(navigator.userAgent)
			|| this.searchVersion(navigator.appVersion)
			|| "an unknown version";
		this.OS = this.searchString(this.dataOS) || "an unknown OS";
	},
	searchString: function (data) {
		for (var i=0;i<data.length;i++)	{
			var dataString = data[i].string;
			var dataProp = data[i].prop;
			this.versionSearchString = data[i].versionSearch || data[i].identity;
			if (dataString) {
				if (dataString.indexOf(data[i].subString) != -1)
					return data[i].identity;
			}
			else if (dataProp)
				return data[i].identity;
		}
	},
	searchVersion: function (dataString) {
		var index = dataString.indexOf(this.versionSearchString);
		if (index == -1) return;
		return parseFloat(dataString.substring(index+this.versionSearchString.length+1));
	},
	dataBrowser: [
		{ 	string: navigator.userAgent,
			subString: "OmniWeb",
			versionSearch: "OmniWeb/",
			identity: "OmniWeb"
		},
		{
			string: navigator.vendor,
			subString: "Apple",
			identity: "Safari"
		},
		{
			prop: window.opera,
			identity: "Opera"
		},
		{
			string: navigator.vendor,
			subString: "iCab",
			identity: "iCab"
		},
		{
			string: navigator.vendor,
			subString: "KDE",
			identity: "Konqueror"
		},
		{
			string: navigator.userAgent,
			subString: "Firefox",
			identity: "Firefox"
		},
		{
			string: navigator.vendor,
			subString: "Camino",
			identity: "Camino"
		},
		{		// for newer Netscapes (6+)
			string: navigator.userAgent,
			subString: "Netscape",
			identity: "Netscape"
		},
		{
			string: navigator.userAgent,
			subString: "MSIE",
			identity: "Explorer",
			versionSearch: "MSIE"
		},
		{
			string: navigator.userAgent,
			subString: "Gecko",
			identity: "Mozilla",
			versionSearch: "rv"
		},
		{ 		// for older Netscapes (4-)
			string: navigator.userAgent,
			subString: "Mozilla",
			identity: "Netscape",
			versionSearch: "Mozilla"
		}
	],
	dataOS : [
		{
			string: navigator.platform,
			subString: "Win",
			identity: "Windows"
		},
		{
			string: navigator.platform,
			subString: "Mac",
			identity: "Mac"
		},
		{
			string: navigator.platform,
			subString: "Linux",
			identity: "Linux"
		}
	]

};
BrowserDetect.init();
		
