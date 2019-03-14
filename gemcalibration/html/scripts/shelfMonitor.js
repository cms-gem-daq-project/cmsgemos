var actionURL = "";
function store_actionURL(t_str) 
{
  actionURL = t_str;
};

function sendrequest( jsonurl )
{
  if (window.jQuery) {
    // can use jQuery libraries rather than raw javascript
    $.getJSON(jsonurl)
      .done(function(data) {
          updateDaqMonitorables( data );
	  console.log("sendrequest:jQuery getJSON succeedeed, status: " + textStatus ); //CG
      })
      .fail(function(data, textStatus, error) {
        console.error("sendrequest: getJSON failed, status: " + textStatus + ", error: "+error);
      });
  } else {
    var xmlhttp;
    if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
      xmlhttp=new XMLHttpRequest();
    } else {// code for IE6, IE5
      xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.onreadystatechange=function()
    {
      if (xmlhttp.readyState==4 && xmlhttp.status==200) {
        var res = eval( "(" + xmlhttp.responseText + ")" );
        console.log("response:"+xmlhttp.responseText);
        console.log("res:"+res);
	console.log("sendrequest:XML getJSON succeedeed, status: " + textStatus ); //CG
        updateDaqMonitorables( res );
      }
    };
    xmlhttp.open("GET", jsonurl, true);
    xmlhttp.send();
  }
};

function updateDaqMonitorables( shelfjson )
{
  for ( var amc in shelfjson ) {
    var monitorset = shelfjson[amc];
    for ( var monitem in monitorset ) {
      try {
        document.getElementById( monitem ).className = monitorset[monitem].class_name;
        document.getElementById( monitem ).innerHTML = monitorset[monitem].value;
      } catch (err) {
        console.error(err.message);
      }
    }
  }
};

function startUpdate( jsonurl )
{
  // here has to be optohybrid web specific
  var interval;
  interval = setInterval( "sendrequest( \"" + jsonurl + "\" )" , 1000 );
};

function expert_action(id)
{ 
    if (window.jQuery) {
    // can use jQuery libraries rather than raw javascript
    $.getJSON(actionURL+id+"Action")
      .done(function(data) {
        document.getElementById( "mon_state" ).innerHTML = data["mon_state"];
	//document.getElementById( "cal_scurve" ).innerHTML = data["cal_scurve"];//CG jQuery
        console.info(" jQuery expert action call succeeded"); //CG jQuery
      })
      .fail(function(data, textStatus, error) {
        console.error("expert action call failed");
      });
    } else {
    var xmlhttp;
    if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
	xmlhttp=new XMLHttpRequest();
	console.info("XML 1expert action call succeeded"); //CG
    } else {// code for IE6, IE5
	xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
	console.info("XML 2expert action call succeeded"); //CG
    }
    xmlhttp.onreadystatechange=function()
    {
      if (xmlhttp.readyState==4 && xmlhttp.status==200) {
        var res = eval( "(" + xmlhttp.responseText + ")" );
        console.log("response:"+xmlhttp.responseText);
        console.log("res:"+res);
	console.info("XML expert action call succeeded"); //CG
        document.getElementById( "mon_state" ).innerHTML = data["mon_state"];
      }
    };
    xmlhttp.open("GET", jsonurl, true);
	xmlhttp.send();
    }
};
function updateCalibMonitorables( shelfjson )
{
    document.getElementById( monitem ).value
  for ( var amc in shelfjson ) {
    var monitorset = shelfjson[amc];
    for ( var monitem in monitorset ) {
      try {
        document.getElementById( monitem ).className = monitorset[monitem].class_name;
        document.getElementById( monitem ).innerHTML = monitorset[monitem].value;
      } catch (err) {
        console.error(err.message);
      }
    }
  }
};
