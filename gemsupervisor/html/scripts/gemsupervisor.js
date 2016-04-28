function sendrequest( jsonurl )
{
    if (window.jQuery) {  
        // can use jQuery libraries rather than raw javascript
        $.getJSON(jsonurl)
            .done(function(data) {
                    updateStatePage( data );
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
                    updateStatePage( res );
                }
            };
        xmlhttp.open("GET", jsonurl, true);
        xmlhttp.send();
    }
};

function updateStatePage( statejson )
{
    //console.log("statejson:"+statejson);
    for ( var set in statejson ) {
        //console.log("set:"+set);
        var arr = statejson[set];
        //console.log("statejson[set]:"+statejson[set]);
        for( var i=0; i<arr.length; i++ ) {
            //console.log("arr[i]:"+arr[i]);
            // make the colour red if the state is Error or Failed?
            document.getElementById( arr[i].name ).innerHTML = "<h3>" + arr[i].value + "</h3>";
        }
    }
};

function startUpdate( jsonurl )
{
    // this should not be too much more than twice the frequency of the infospace update interval
    var interval;
    interval = setInterval( "sendrequest( \"" + jsonurl + "\" )" , 5000 );
};
