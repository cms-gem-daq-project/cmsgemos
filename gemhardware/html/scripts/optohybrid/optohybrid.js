// copied directly from ferol.js, try to implement their ajax style page updates
function pdebug( text )
{
    // here has to be optohybrid web specific
    document.getElementById("debug").innerHTML = text;
};

function sendrequest( jsonurl )
{
    if (window.jQuery) {
        // can use jQuery libraries rather than raw javascript
        $.getJSON(jsonurl)
            .done(function(data) {
                    updateOptoHybridMonitorables( data );
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
                    updateOptoHybridMonitorables( res );
                }
            };
        xmlhttp.open("GET", jsonurl, true);
        xmlhttp.send();
    }
};

function updateOptoHybridMonitorables( ohjson )
{
    for ( var oh in ohjson ) {
        var monitorset = ohjson[oh];
        for ( var monitem in monitorset ) {
            var arr = monitorset[monitem];
            for( var i = 0; i < arr.length; ++i ) {
                try {
                    document.getElementById( arr[i].name ).innerHTML = arr[i].value;
                } catch (err) {
                    console.error(err.message);
                }
            }
        }
    }
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

function expandedDescription( description )
{
    //console.log( description );
    // anchor text box to cursor position
    // get selected td and modify contents?

    $(document).ready(function() {
        var changeTooltipPosition = function(event) {
            console.log("changeTooltipPosition called");
            var tooltipX = event.pageX - 8;
            var tooltipY = event.pageY + 8;
            $('div.tooltip').css({top: tooltipY, left: tooltipX});
        };

        var showTooltip = function(event) {
            console.log("showTooltip called");
            $('div.description').remove();
            $('<div class="description">'+description+'</div>')
            .appendTo('body');
            changeTooltipPosition(event);
        };

        var hideTooltip = function() {
            console.log("hideTooltip called");
            $('div.description').remove();
        };

        $("td#description").bind({
            mousemove  : changeTooltipPosition,
            mouseenter : showTooltip,
            mouseleave : hideTooltip
        });
    });
};

/* copied from oh/oh.js
function pad( width, string, padding )
{
    console.log(width + " - " + string + " - " + padding );
    return (width <= string.length) ? string : pad(width, padding + string, padding);
};
*/
