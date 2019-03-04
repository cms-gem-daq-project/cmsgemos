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
        console.info("expert action call succeeded");
      })
      .fail(function(data, textStatus, error) {
        console.error("expert action call failed");
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
        document.getElementById( "mon_state" ).innerHTML = data["mon_state"];
      }
    };
    xmlhttp.open("GET", jsonurl, true);
    xmlhttp.send();
  }
};
