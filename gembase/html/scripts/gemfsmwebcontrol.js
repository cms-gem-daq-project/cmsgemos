function fsmdebug( text )
{
    document.getElementById("fsmdebug").innerHTML = text;
};

function showTable( jsonurl )
{
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
            for ( var set in res ) {
                //console.log("set:"+set);
                var arr = res[set];
                //console.log("res[set]:"+res[set]);
                for( var i=0; i<arr.length; i++ ) {
                    //console.log("arr[i]:"+arr[i]);
                    // make the colour red if the state is Error or Failed?
                    document.getElementById( arr[i].name ).innerHTML = "<h3>" + arr[i].value + "</h3>";
                }
            }
        }
    };
    xmlhttp.open("GET", jsonurl, true);
    xmlhttp.send();

    // why is this "undefined"?
    //var state = $("h3#fsmState").text;
    // but this one works...
    var state = document.getElementById("fsmState").innerHTML;
    //console.log("html" + $("h3#fsmState").html);
    //console.log("text" + $("h3#fsmState").text);
    //console.log("innerHTML" + $("h3#fsmState").innerHTML);
    console.log("building the status table, state is " + state);
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
    var interval;
    interval = setInterval("showTable(" + statejson +")", 1000);
}

function gemFSMWebCommand( command )
{
    $("input#fsmcommand").val( command );
    fsmdebug( "setting command to " + command );
    $("form#fsmControl").submit();
};
