$(document).ready( function () {
	var registerSetBoxes = [$("#CR0Set"),$("#CR1Set"),$("#CR2Set"),$("#CR3Set"),
				$("#SetIPreampIn"),$("#SetIPreampFeed"),$("#SetIPreampOut"),
				$("#SetIShaper"),$("#SetIShaperFeed"),
				$("#SetIComp"),
				$("#SetLatency"),
				$("#SetVCal"),
				$("#SetVThreshold1"),$("#SetVThreshold2"),
				$("#SetCalPhase")
				];
	var nBoxes = registerSetBoxes.length;
	
	$("#SelectAll").click(function(event) {
		for (var i=0; i<nBoxes; ++i ) {
		    registerSetBoxes[i].get(0).checked=true;
		};
	    });
	$("#SelectNone").click(function(event) {
		for (var i=0; i<nBoxes; ++i ) {
		    registerSetBoxes[i].get(0).checked=false;
		};
	    });
    });
