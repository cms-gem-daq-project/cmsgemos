function sendrequest( jsonurl )
{
    if (window.jQuery) {
        // can use jQuery libraries rather than raw javascript
        $.getJSON(jsonurl)
            .done(function(data) {
                    updateGLIBMonitorables( data );
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
                    updateGLIBMonitorables( res );
                }
            };
        xmlhttp.open("GET", jsonurl, true);
        xmlhttp.send();
    }
};

function updateGLIBMonitorables( glibjson )
{
    for ( var glib in glibjson ) {
        var monitorset = glibjson[glib];
        for ( var monitem in monitorset ) {
            var arr = monitorset[monitem];
            for( var i = 0; i < arr.length; ++i ) {
                document.getElementById( arr[i].name ).innerHTML = arr[i].value;
            }
        }
    }
};

function startUpdate( jsonurl )
{
    // this should not be too much more than twice the frequency of the infospace update interval
    var interval;
    interval = setInterval( "sendrequest( \"" + jsonurl + "\" )" , 5000 );
};

function dumpGLIBTrackingData( command, urn )
{
    var jsonurl = urn + "/" + command;
    console.log( jsonurl );

    if (window.jQuery) {
        // can use jQuery libraries rather than raw javascript
        $.getJSON(jsonurl)
            .done(function(data) {
                    console.log( $("input#glibID").val()  );
                    console.log( $("input#gtxlink").val() );
                    showFIFODump( data, $("input#glibID").val(), $("input#gtxlink").val() );
                })
            .fail(function(data, textStatus, error) {
                    console.error("dumpGLIBTrackingData: getJSON failed, status: " + textStatus + ", error: "+error);
                });
    } else {
        var xmlhttp;
        console.log( document.getElementById("glibID").value );
        console.log( document.getElementById("gtxlink").value );
        if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
            xmlhttp=new XMLHttpRequest();
        } else {// code for IE6, IE5
            xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
        }
        xmlhttp.onreadystatechange=function()
            {
                if (xmlhttp.readyState==4 && xmlhttp.status==200) {
                    var res = eval( "(" + xmlhttp.responseText + ")" );
                    showFIFODump( res, document.getElementById("glibID").value, document.getElementById("gtxlink").value );
                }
            };
        xmlhttp.open("GET", jsonurl, true);
        xmlhttp.send();
    }
};

function showFIFODump( fifodump, glibID, gtxLink )
{
    console.log("fifodump:"+fifodump);
    var myglib = "glib"+pad(2,glibID,"0")+"FIFO";
    for ( var set in fifodump ) {
        console.log("set:"+set);
        if (set === myglib) {
            console.log(myglib);
            var arr = fifodump[set];
            console.log("fifodump[set]:"+fifodump[set]);
            console.log("arr.value:"+arr.value);
            var dump = arr.value.join(' ');
            dump.replace(/\n/g, "&#10;");
            console.log("dump:"+dump);
            document.getElementById( "glibtrackingdata" ).innerHTML = dump;
        }
    }
};

function pad( width, string, padding )
{
    console.log(width + " - " + string + " - " + padding );
    return (width <= string.length) ? string : pad(width, padding + string, padding);
};
