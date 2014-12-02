$(document).ready( function () {
    });

function changeImage (form) {
    var histNum = form.ChannelHist.value;
    //alert("Selecting channel "+histNum+" histogram");
    document.getElementById("vfatChannelHisto").src="/gemdaq/gemsupervisor/html/images/tbutils/tscan/chanthresh"+histNum+".png";
};
