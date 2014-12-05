<%@ page language="java" contentType="text/html" %>
<%@ page import="java.util.concurrent.atomic.AtomicInteger" %>

<%
    response.setHeader("Cache-Control","no-cache"); //HTTP 1.1
    response.setHeader("Pragma","no-cache"); //HTTP 1.0
    response.setDateHeader ("Expires", 0); //prevents caching at the proxyserver
%>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<%@page import="confdb.converter.OnlineConverter"%>
<%@page import="confdb.db.ConfDB"%>
<%@page import="confdb.data.Directory"%>
<%@page import="confdb.data.ConfigInfo"%>
<%@page import="java.io.ByteArrayOutputStream"%>
<%@page import="java.io.PrintWriter"%>
<html>

<head>
	<META Http-Equiv="Cache-Control" Content="no-cache">
	<META Http-Equiv="Pragma" Content="no-cache">
	<META Http-Equiv="Expires" Content="0">
	
	<title>HLT config tree</title>

	<link rel="StyleSheet" href="../../gui/css/dtree.css" type="text/css" />
	<link rel="StyleSheet" href="../../gui/css/common.css" type="text/css" />
	<script type="text/javascript" src="../../gui/js/dtree.js"></script>
	<script type="text/javascript" src="../../gui/js/common.js"></script>
	<script type="text/javascript" src="../js/myControl.js"></script>
	
<script>

var configName;
var configVersion;
var configKey;

function onLoad() 
{
}
		
function onClickOpenAllNodes() 
{
  d.openAll();
}
		
function onClickCloseAllNodes() 
{
  d.closeAll();
}
		
function showSelectedConfig( name, version, id )
{
  configName = name;
  configVersion = version;
  configKey = id;
  document.getElementById( "selectedConfig" ).innerHTML = "<b>" + name + " V" + version + "</b>";
  document.getElementById("okButton").style.visibility = "visible";
  document.getElementById("config").src = "convert.jsp?configKey=" + id;
}

function onClickSend()
{
 // window.close();
 // var config = configName + " V" + configVersion;
 // opener.alert( config );
 // opener.document.getElementById("currentConfig").innerHTML = config;
  var description = configName + " V" + configVersion;
  setRunDataParameter("HLT_KEY","rcms.fm.fw.parameter.type.IntegerT",configKey,
 					  "HLT_KEY_DESCRIPTION","rcms.fm.fw.parameter.type.StringT", description);
  window.close();
  
}

function setRunDataParameter(formName1,type1,value1,formName2,type2,value2)
{
    // set the new value in the form element and update the
    // FM.
	document.FMPilotForm.globalParameterName3.value=formName1;
	document.FMPilotForm.globalParameterValue3.value=value1;
	document.FMPilotForm.globalParameterType3.value=type1;
	
	document.FMPilotForm.globalParameterName4.value=formName2;
	document.FMPilotForm.globalParameterValue4.value=value2;
	document.FMPilotForm.globalParameterType4.value=type2;
	
	submitForm('FMPilotForm', 'SET_GLOBAL_PARAMETERS');   
	
	// reset selection to avoid that with the next POST the same command is send
	document.FMPilotForm.globalParameterName3.value="x";
	document.FMPilotForm.globalParameterValue3.value="x";
	document.FMPilotForm.globalParameterType3.value="x";	
	
	document.FMPilotForm.globalParameterName4.value="x";
	document.FMPilotForm.globalParameterValue4.value="x";
	document.FMPilotForm.globalParameterType4.value="x";
}

function submitForm(idForm, action) {
	var form = document.getElementById(idForm);
	form.ACTION.value = action;
	form.submit();
}
</script>
</head>

<%!
    String prepareTree( AtomicInteger nodeCounter, int parentNode, Directory directory )
    {
      String str = "";
	  ConfigInfo[] configs = directory.listOfConfigurations();

      for ( int i = 0; i < configs.length; i++ )
      {
        confdb.data.ConfigVersion versionInfo = configs[i].version( 0 ); 
		String name = configs[i].name();
		str += "d.add(" + nodeCounter.incrementAndGet() + ", " + parentNode + ", '" + name 
	     + "','javascript:showSelectedConfig( \\'" + name + "\\', " 
	     + versionInfo.version() + ", " + versionInfo.dbId() 
             + ");', '', '', '../img/page.gif', '../img/page.gif'  );\n";
		int thisParent = nodeCounter.get();	 
		for ( int ii = 0; ii < configs[i].versionCount(); ii++ )
        {
	  	  versionInfo = configs[i].version( ii );
	  	  String vx = "V" + versionInfo.version() + "  -  " + versionInfo.created();
	  	  str += "d.add(" + nodeCounter.incrementAndGet() + ", " + thisParent + ", '" + vx 
	          + "','javascript:showSelectedConfig( \\'" + name + "\\', " 
	          + versionInfo.version() + ", " + versionInfo.dbId() + ");', '', '', '../img/spacer.gif' )\n";
        }
      }
  	  Directory[] list = directory.listOfDirectories();
      for ( int i = 0; i < list.length; i++ )
      {
		str += "d.add(" + nodeCounter.incrementAndGet() + ", " + parentNode + ", '" + list[i].name() 
            + "','javascript:d.openTo(" + nodeCounter.get() +",false);');\n";
        str += prepareTree( nodeCounter, nodeCounter.get(), list[i] );
      }
      return str;
    }
%>
<body onLoad="onLoad()">
<p> groupID: <%= request.getParameter("groupID") %>
<table width="100%" border="1" cellpadding="0" cellspacing="0" bordercolor="#CC6600" bgcolor="#EEEEEE">
<tbody id="treeSection">
<tr>
<%
  String tree = "";
  try {
	OnlineConverter converter = OnlineConverter.getConverter();
	ConfDB confDB = converter.getDatabase();
    Directory root = confDB.loadConfigurationTree();
    tree = "<script>\n" 
        + "d = new dTree('d');\n"
        + "d.add(0,-1,'HLT configurations','javascript: void(0);');\n"
        + prepareTree( new AtomicInteger(1), 0, root )
        + "document.write(d);\n"
	+ "</script>\n";
  } catch (Exception e) {
	  ByteArrayOutputStream buffer = new ByteArrayOutputStream();
	  PrintWriter writer = new PrintWriter( buffer );
	  Throwable cause = e.getCause();
	  if ( cause != null )
	  {
		  writer.println( e );
		  cause.printStackTrace( writer );
	  }
	  else
		  e.printStackTrace( writer );
	  writer.close();
	  out.println( "<tr><td>" + buffer.toString() + "</td></tr>" );
  }
%>
<td class="label_center" >
 <input type="button" id="openAllNodes" value="Open All Nodes" name="Open All Nodes" onClick="onClickOpenAllNodes()">
 <input type="button" id="closeAllNodes" value="Close All Nodes" name="Close All Nodes" onClick="onClickCloseAllNodes()">
</td>
<td>selected: 
 <span id="selectedConfig"></span>
 <span id="okButton" style="text-align:right; margin-left:10px; visibility:hidden"><input type="button" value="OK" onClick="onClickSend()"></span>
</td>
</tr>
<tr>
<td valign="top">
<%= tree %>
</td>
<td>
<iframe id="config" width="100%" height="650" marginheight="10" marginwidth="10" frameborder="0">
</iframe>
</td>
</tr>
</tbody>
</table>

<form name="FMPilotForm" id="FMPilotForm" method="post" action="../../gui/servlet/FMPilotServlet">
<input type="hidden" id="globalParameterName3 " name="globalParameterName3" value=""  /> 
<input type="hidden" id="globalParameterValue3" name="globalParameterValue3"  value="" />		
<input type="hidden" id="globalParameterType3" name="globalParameterType3"  value="" />		
<input type="hidden" id="globalParameterName4 " name="globalParameterName4" value=""  /> 
<input type="hidden" id="globalParameterValue4" name="globalParameterValue4"  value="" />		
<input type="hidden" id="globalParameterType4" name="globalParameterType4"  value="" />		
<input type="hidden" id="ACTION" name="ACTION" value="SET_GLOBAL_PARAMETERS">
<input type="hidden" id="COMMAND" name="COMMAND" value=""> 
<input type="hidden" id="TIMESTAMP" name="TIMESTAMP" value="">
<input type="hidden" id="groupID" name="groupID" value="<%= request.getParameter("groupID") %>">
</form>

</body>

</html>

