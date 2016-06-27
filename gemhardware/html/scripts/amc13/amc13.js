$(document).ready( function() {
        if (window.jQuery) {
            $( ".amc13statuslevel" ).change(function() {
                    var requestURL = $('#amc13statusform').prop("action");
                    $.post(requestURL, $('#amc13statusform').serialize());
                    updateURL = requestURL.replace("setDisplayLevel","updateStatus");
                    sendrequest( updateURL );
                });
        }
    });

function sendrequest( texturl )
{
    if (window.jQuery) {
        // can use jQuery libraries rather than raw javascript
        $.get(texturl)
            .done(function(data) {
                    updateAMC13StatusPage( data );
                })
            .fail(function(data, textStatus, error) {
                    console.error("sendrequest: get failed, status: " + textStatus + ", error: "+error);
                    console.log( data );
                    updateAMC13StatusPage( data.responseText );
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
                    updateAMC13StatusPage( res );
                }
            };
        xmlhttp.open("GET", texturl, true);
        xmlhttp.send();
    }
};

function updateAMC13StatusPage( amc13status )
{
    document.getElementById( "amc13status" ).innerHTML = amc13status;
};

function startUpdate( updateurl )
{
    // this should not be too much more than twice the frequency of the infospace update interval
    var interval;
    interval = setInterval( "sendrequest( \"" + updateurl + "\" )" , 5000 );
};

