$(document).ready( function () {
    });

function changeImage (form) {
    var histNum = form.ChannelHist.value;
    //alert("Selecting channel "+histNum+" histogram");
    document.getElementById("vfatChannelHisto").src="/gemdaq/gemsupervisor/baranov/images/tbutils/tscan/chanthresh"+histNum+".png";
};
