$(document).ready( function () {
	/*
	var imageDir = "/gemdaq/gemsupervisor/html/images/tbutils/tscan/";
	//var histNum = "0";
	
	var intervalMS = 2000; // 2 seconds
	var imgSrc = document.getElementById("vfatChannelHisto").src;
	//alert(imgSrc);
	setInterval(function() {
		document.getElementById("vfatChannelHisto").src=imgSrc+"?"+(new Date().valueOf());
		//$("#vfatChannelHisto").attr("src", imgDir+"/chanthresh"+histNum+".png?" + new Date().valueOf());
	    },intervalMS);
	*/
    });

function changeImage (form) {
    var imageDir = "/gemdaq/gemsupervisor/html/images/tbutils/tscan/";
    var histNum = "0";
    var intervalMS = 2000; // 2 seconds
    var imgSrc = document.getElementById("vfatChannelHisto").src;
    histNum = form.ChannelHist.value;
    document.getElementById("vfatChannelHisto").src=imageDir+"/chanthresh"+histNum+".png?"+(new Date().valueOf());
    document.getElementById("ChannelHist").value=histNum;
    var imgSrc = document.getElementById("vfatChannelHisto").src;
};

