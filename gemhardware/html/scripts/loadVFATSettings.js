$(document).ready(function() {
	$('#ReadCounters').click(function(e) {
		e.preventDefault(); // prevent the page to do the usuall onclick event (from reloading the page)
		// navigate to Home;
	    });
	$('#ScanForVFATS').click(function(e) {
		e.preventDefault();
		// return true;
	    });
	$('#InitializeVFAT').click(function(e) {
		e.preventDefault();
		// return true;
	    });
	$('#Read').click(function(e) {
		e.preventDefault();
		// return true;
	    });
	$('#Write').click(function(e) {
		e.preventDefault();
		// return true;
	    });
	$('#SelectAll').click(function(e) {
		e.preventDefault();
		// return true;
	    });
	$('#SelectNone').click(function(e) {
		e.preventDefault();
		// return true;
	    });
	$('#Load').click(function(e) {
		e.preventDefault();
		// return true;
	    });
	$('#Export').click(function(e) {
		e.preventDefault();
		// return true;
	    });
	$('#GetChannel').click(function(e) {
		e.preventDefault();
		// return true;
	    });
	$('#SetChannel').click(function(e) {
		e.preventDefault();
		// return true;
	    });
	$('#SetAllChannels').click(function(e) {
		e.preventDefault();
		// return true;
	    });
    });
$(function(){
	$(".vfatButtonInput").click(function(e){
		alert($(this).attr("name"));
	    });
    });

$(function(){
	$(".vfatChannelButton").click(function(e){
		alert($(this).attr("name"));
	    });
    });

/*
  $(function(){
  $.ajax({
  type: "GET",
  url: "{$BUILD_HOME}/${GEM_OS_PROJECT}/gemhardware/xml/vfat/vfat_settings.xml",
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

	
  <input class="vfatButtonInput" type="submit" value="Read counters" name="ReadCounters"  />
  <input class="vfatButtonInput" type="submit" value="Scan for connected VFATs" name="ScanForVFATS"  />
  <input class="vfatButtonInput" type="submit" value="Initialize VFAT" name="InitializeVFAT"  />
  <input class="vfatButtonInput" type="submit" value="Read" name="Read"  />
  <input class="vfatButtonInput" type="submit" value="Write" name="Write"  />
  <input class="vfatButtonInput" type="submit" value="Select All Registers" name="SelectAll"  />
  <input class="vfatButtonInput" type="submit" value="Select None" name="SelectNone"  />
  <input class="vfatButtonInput" type="submit" value="Load XML settings" name="Load"  />
  <input class="vfatButtonInput" type="submit" value="Export current settings" name="Export"  />

  <input class="vfatChannelButton" type="submit" value="Get This Channel" name="GetChannel"  />
  <input class="vfatChannelButton" type="submit" value="Set This Channel" name="SetChannel"  /><br />
  <input class="vfatChannelButton" type="submit" value="Set All Channels" name="SetAllChannels"  />

  });
*/
