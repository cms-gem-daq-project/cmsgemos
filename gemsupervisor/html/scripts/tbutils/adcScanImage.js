$(document).ready( function () {
	var imageDir = "/gemdaq/gemsupervisor/html/images/tbutils/dacscan/";
	var intervalMS = 2000; // 2 seconds
	var imgName = document.getElementById("vfatChannelHisto").name;
	setInterval(function() {
		//alert("image "+imgName+" updated");
		document.getElementById("vfatChannelHisto").src=imageDir+"/"+imgName+"?"+(new Date().valueOf());
		//$("#vfatChannelHisto").attr("src", imageDir+"/"+imgName+"?" + new Date().valueOf());
	    },intervalMS);
    });

