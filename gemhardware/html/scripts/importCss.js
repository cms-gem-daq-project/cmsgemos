$(document).ready(function() {
	var $head = $("head");
	var $headlinklast = $head.find("link[rel='stylesheet']:last");
	if ($headlinklast.length){
	    $headlinklast.after("<link rel='stylesheet' href='gemdaq/gemhardware/html/css/vfat/vfatmanager.css' type='text/css'>";
);
	}
	else {
	    $head.append(linkElement);
	}
	<link rel="stylesheet" href="gemdaq/gemhardware/html/css/vfat/vfatmanager.css"/>;
	<link rel="stylesheet" href="gemdaq/gemhardware/html/css/vfat/vfatcontrolregisters.css"/>;
	<link rel="stylesheet" href="gemdaq/gemhardware/html/css/vfat/vfatglobalsettings.css"/>;
	<link rel="stylesheet" href="gemdaq/gemhardware/html/css/vfat/vfatchannelregister.css"/>;
	<link rel="stylesheet" href="gemdaq/gemhardware/html/css/vfat/vfatcommands.css"/>;

    });