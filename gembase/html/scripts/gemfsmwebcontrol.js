function fsmdebug( text )
{
    document.getElementById("fsmdebug").innerHTML = text;
};


function gemFSMWebCommand( command )
{
    $("input#fsmcommand").val( command );
    fsmdebug( "setting command to " + command );
    $("form#fsmControl").submit();
};
