function fsmdebug( text )
{
    document.getElementById("fsmdebug").innerHTML = text;
};

function staterequest( jsonurl )
{
    if (window.jQuery) {  
        //console.log("jQuery is loaded");
        // can use jQuery libraries rather than raw javascript
    } else {
        //console.log("jQuery is not loaded");
    }
    //console.log("staterequest(\"" + jsonurl + "\")");
    // need to update the state in the state table with AJAX
    var xmlhttp;
    if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp=new XMLHttpRequest();
    } else {// code for IE6, IE5
        xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.onreadystatechange=function()
        {
            if (xmlhttp.readyState==4 && xmlhttp.status==200) {
                //console.log("response:"+xmlhttp.responseText);
                var res = eval( "(" + xmlhttp.responseText + ")" );
                //console.log("res:"+res);
                //console.log("res.name:"+res.name);
                //console.log("res.value:"+res.value);
                
                //document.getElementById( res.name ).innerHTML = "<h3>" + res.value + "</h3>";
                document.getElementById( res.name ).innerHTML = res.value;
            }
        };
    //console.log("jsonurl:"+jsonurl);
    xmlhttp.open("GET", jsonurl, true);
    xmlhttp.send();
}

function showTable( )
{
    //console.log("showTable()");
    document.getElementById("fsmdebug").innerHTML = document.getElementById("fsmdebug").innerHTML;
    var state = document.getElementById("fsmState").innerHTML;
    //console.log("building the status table, state is " + state);
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
    //console.log("updateStateTable( \""+statejson+"\" )");
    var interval;
    interval = setInterval(" staterequest(\"" + statejson +"\" )", 1000);
    interval = setInterval(" showTable()", 1000);
}

function gemFSMWebCommand( command, url )
{
    //console.log("gemFSMWebCommand( \""+command+"\" )");
    // want to hide the control table when the button is pressed
    $("tr.hide#initconf" ).hide();
    $("tr.hide#startstop").hide();
    $("tr.hide#haltreset").hide();
    fsmdebug( command );
    var xmlhttp;
    if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp=new XMLHttpRequest();
    } else {// code for IE6, IE5
        xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.open("POST", url+"/"+command, true);
    xmlhttp.send();
    staterequest( url+"/stateUpdate" );
    $("tr.hide#initconf" ).hide();
    $("tr.hide#startstop").hide();
    $("tr.hide#haltreset").hide();
    showTable();
};

