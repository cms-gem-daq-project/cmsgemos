function fsmdebug( text )
{
    document.getElementById("fsmdebug").innerHTML = text;
};

function staterequest( jsonurl )
{
    if (window.jQuery) {  
        $.getJSON(jsonurl)
            .done(function(data) {
                    updateStateMonitorables( data );
                })
            .fail(function(data, textStatus, error) {
                    console.error("staterequest: getJSON failed, status: " + textStatus + ", error: "+error);
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
                    updateStateMonitorables( res );
                }
            };
        xmlhttp.open("GET", jsonurl, true);
        xmlhttp.send();
    }
}

function updateStateMonitorables( statejson ) 
{
    document.getElementById( statejson.name ).innerHTML = statejson.value;
};

function showTable( )
{
    document.getElementById("fsmdebug").innerHTML = document.getElementById("fsmdebug").innerHTML;
    var state = document.getElementById("fsmState").innerHTML;
    if (state.indexOf("ing") > 0 && state != "Running") {
        $("tr.hide#initconf" ).hide();
        $("tr.hide#startstop").hide();
        $("tr.hide#haltreset").hide();
    } else if (state == "Initial") {
        // show only Initialize button
        $("tr.hide#initconf").show();
        $("button.hide#init").show();
        $("button.hide#conf").hide();
        // hide startstop tr
        $("tr.hide#startstop").hide();
        // hide haltreset tr
        $("tr.hide#haltreset").hide();
    } else {
        // hide Initialize button
        $("tr.hide#initconf").show();
        $("button.hide#init").hide();
        $("button.hide#conf").show();
        
        // startstop tr
        if (state == "Halted") {
            // hide startstop tr
            $("tr.hide#startstop").hide();
        } else if (state == "Configured") {
            // hide stop/pause/resume buttons
            $("tr.hide#startstop" ).show();
            $("button.hide#start" ).show();
            $("button.hide#stop"  ).hide();
            $("button.hide#pause" ).hide();
            $("button.hide#resume").hide();
        } else if (state == "Running") {
            // hide stop/resume buttons
            $("tr.hide#startstop" ).show();
            $("button.hide#start" ).hide();
            $("button.hide#stop"  ).show();
            $("button.hide#pause" ).show();
            $("button.hide#resume").hide();
        } else if (state == "Paused") {
            // hide pause button
            $("tr.hide#startstop" ).show();
            $("button.hide#start" ).hide();
            $("button.hide#stop"  ).show();
            $("button.hide#pause" ).hide();
            $("button.hide#resume").show();
        }
        
        // haltreset tr
        if (state == "Halted"  || state == "Configured" ||
            state == "Running" || state == "Paused") {
            // show halt/reset buttons
            $("tr.hide#haltreset").show();
            $("button.hide#halt" ).show();
            $("button.hide#reset").show();
        } else if (state == "Failed" || state == "Error") {
            // hide halt button
            $("tr.hide#haltreset").show();
            $("button.hide#halt" ).hide();
            $("button.hide#reset").show();
        }
    }// state not initial
} // end showTable function

function updateStateTable( statejson )
{
    var interval;
    interval = setInterval(" staterequest(\"" + statejson +"\" )", 1000);
    interval = setInterval(" showTable()", 500);
}

function gemFSMWebCommand( command, url )
{
    // want to hide the control table when the button is pressed
    $("tr.hide#initconf" ).hide();
    $("tr.hide#startstop").hide();
    $("tr.hide#haltreset").hide();
    fsmdebug( command );

    var requestURL = url+"/"+command;
    var options = {
        headers: {},
        url: requestURL,
        data: null
    };
    xdaqAJAX(options,null);

    staterequest( url+"/stateUpdate" );
    $("tr.hide#initconf" ).hide();
    $("tr.hide#startstop").hide();
    $("tr.hide#haltreset").hide();
    showTable();
};

