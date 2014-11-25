$(document).ready( function () {
	alert("loaded change image script");
	function changeImage (form) {
	    var histNum = form.ChannelHist.value;
	    alert("Selecting channel "+histNum+" histogram");
	    document.getElementById("vfatChannelHisto").src="/gemdaq/gemsupervisor/html/images/tbutils/tscan/channel"+histNum+".png";
	});
    });
