$(function(){
	$.ajax({
		type: "GET",
		url: "$BUILD_HOME/$GEM_OS_PROJECT/gemhardware/xml/vfat/vfat_settings.xml",
		dataType: "xml",
		success: function(xml) {
		    //remainder of the code
		}
		$(xml).find('car').each(function(){
			var Titles = $(this).find('Title').text();
			var Manufacturers = $(this).find('Manufacturer').text();
			$("<li></li>").html(Titles + ", " + Manufacturers).appendTo("#Autom ul");
		});
	});
	error: function() {
	    alert("The XML File could not be processed correctly.");
	}
});
