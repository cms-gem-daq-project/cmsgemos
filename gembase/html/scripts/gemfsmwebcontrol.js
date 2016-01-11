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
    // why is this "undefined"?
    //var state = $("h3#fsmState").text;
    // but this one works...
    document.getElementById("fsmdebug").innerHTML = document.getElementById("fsmdebug").innerHTML;
    var state = document.getElementById("fsmState").innerHTML;
    //console.log("html" + $("h3#fsmState").html);
    //console.log("text" + $("h3#fsmState").text);
    //console.log("innerHTML" + $("h3#fsmState").innerHTML);
    //console.log("building the status table, state is " + state);
    if (state == "Initial") {
        // show only Initialize button
        //console.log("hiding the conf button");
        $("button.hide#conf").hide();
        // hide startstop tr
        //console.log("hiding the startstop tr");
        $("tr.hide#startstop").hide();
        // hide haltreset tr
        //console.log("hiding the haltreset tr");
        $("tr.hide#haltreset").hide();
    } else {
        // hide Initialize button
        //console.log("hiding the halt button");
        $("button.hide#init").hide();
        
        // startstop tr
        if (state == "Halted") {
            // hide startstop tr
            //console.log("hiding the startstop tr");
            $("tr.hide#startstop").hide();
        } else if (state == "Configured") {
            // hide stop/pause/resume buttons
            //console.log("hiding the stop/pause/resume buttons");
            $("button.hide#stop").hide();
            $("button.hide#pause").hide();
            $("button.hide#resume").hide();
        } else if (state == "Running") {
            // hide stop/resume buttons
            //console.log("hiding the stop/resume buttons");
            $("button.hide#stop").hide();
            $("button.hide#resume").hide();
        } else if (state == "Paused") {
            // hide pause button
            //console.log("hiding the pause button");
            $("button.hide#pause").hide();
        }
        // haltreset tr
        if (state == "Halted"  || state == "Configured" ||
            state == "Running" || state == "Paused") {
            //console.log("showing the halt/reset buttons");
            // show halt/reset buttons
        } else if (state == "Failed" || state == "Error") {
            // hide halt button
            //console.log("hiding the halt button");
            $("button.hide#halt").hide();
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

function gemFSMWebCommand( command )
{
    //console.log("gemFSMWebCommand( \""+command+"\" )");
    $("input#fsmcommand").val( command );
    fsmdebug( "setting command to " + command );
    $("form#fsmControl").submit();
};
