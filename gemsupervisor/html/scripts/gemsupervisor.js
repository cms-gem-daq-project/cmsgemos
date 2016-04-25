function sendrequest( jsonurl )
{
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
};

function startUpdate( jsonurl )
{
    // this should not be too much more than twice the frequency of the infospace update interval
    var interval;
    interval = setInterval( "sendrequest( \"" + jsonurl + "\" )" , 5000 );
};
