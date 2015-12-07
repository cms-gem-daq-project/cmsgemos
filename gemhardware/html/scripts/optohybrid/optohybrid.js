// copied directly from ferol.js, try to implement their ajax style page updates
function pdebug( text )
{
    // here has to be optohybrid web specific
    document.getElementById("debug").innerHTML = text;
};

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
                var res = eval( "(" + xmlhttp.responseText + ")" );
                for ( var set in res ) {
                    var arr = res[set];
                    for( var i=0; i<arr.length; i++ ) {
                        document.getElementById( arr[i].name ).innerHTML = arr[i].value;
                    }
                }
            }
        };
    xmlhttp.open("GET", jsonurl, true);
    xmlhttp.send();
};

function startUpdate( jsonurl )
{
    // here has to be optohybrid web specific
    var interval;
    interval = setInterval( "sendrequest( \"" + jsonurl + "\" )" , 1000 );
    
};

function optohybridWebCommand( command )
{   
    // here has to be optohybrid web specific
    $("input#command").val( command );
    pdebug( "setting command to " + command );
    $("form#debugForm").submit();
    //return -1;
};