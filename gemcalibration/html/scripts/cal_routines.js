var actionURL = "";
var cal_type = "";
function store_actionURL(t_str) 
{
  actionURL = t_str;
};

function selectCalType()
{
	console.log("selectCalType: sending AJAX");
    $.ajax({
        url: actionURL+"setCalType",
        data: $('form#cal_select').serialize(),
        type:  'GET',
        dataType: 'html',
        success: function(response)
        {
	        console.log("selectCalType: request succeeded, response below");
            var result = $('<div />').append(response).find("#cal_interface").html();
            console.log(result);
            $('#cal_interface').html(result);
        }
    });
};

function apply_action(id)
{
	console.log("apply_action(): sending AJAX");
    var forms = $('#trigger_select', '#n_samples_select', 'slot_and_masks_select', 'scurve_input_params').serialize();
    console.log(forms);
    $.ajax({
        url: actionURL+"applyAction",
        //data: forms,
        data: $('form').serialize(),
        type:  'POST',
        success: function(data) {
            alert("Parameters applied. Now you can run the scan."); 
            $('#run_button').removeAttr('disabled');
        }
    });
};

function select_links()
{
    if (window.jQuery) {
        //alert("Well... This is still work in progress... But it should launch!");
        var checkboxes = $('form#slot_and_masks_select').find(':checkbox');
        checkboxes.attr('checked', 'checked');
        var masks = $('form#slot_and_masks_select').find(':input');
        masks.attr('value','0x3FF');
    }
};

function run_scan()
{
    if (window.jQuery) {
        alert("Well... This is still work in progress... But it should launch!");
	//$.post(actionURL+"applyAction", $('form#scurve_input_params').serialize())
	//    .done(alert("Applied"))
    }
};


