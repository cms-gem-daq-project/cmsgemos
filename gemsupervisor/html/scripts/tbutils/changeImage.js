$(document).ready( function () {
<<<<<<< HEAD
	function changeImage (form) {
	    var histNum = form.ChannelHist.value;
	    alert("Selecting channel "+histNum+" histogram");
	    document.getElementById("vfatChannelHisto").src="/gemdaq/gemsupervisor/baranov/images/tbutils/tscan/channel"+histNum+".png";
	});
=======
	//alert("loaded change image script");
>>>>>>> b03ef9e2d39fa0982b0314a5469eae16aeea7c19
    });
function changeImage (form) {
    var histNum = form.ChannelHist.value;
    //alert("Selecting channel "+histNum+" histogram");
    document.getElementById("vfatChannelHisto").src="/gemdaq/gemsupervisor/html/images/tbutils/tscan/chanthresh"+histNum+".png";
};
