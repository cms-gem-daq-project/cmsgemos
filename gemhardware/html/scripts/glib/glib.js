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
