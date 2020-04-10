var actionURL = "";
var cal_type = "";
function store_actionURL(t_str)
{
  actionURL = t_str;
};

function selectCalType()
{
    $.ajax({
        url: actionURL+"setCalType",
        data: $('form#cal_select').serialize(),
        type:  'GET',
        dataType: 'html',
        success: function(response)
        {
            var result = $('<div />').append(response).find("#cal_interface").html();
            $('#cal_interface').html(result);
        }
    });
};

function apply_action()
{
    $.ajax({
        url: actionURL+"applyAction",
        data: $('form').serialize(),
        type:  'POST',
        success: function(data) {
            if (data["status"] == 0) {
                alert(data["alert"]);
                $('#run_button').removeAttr('disabled');
            } else {
                alert(data["alert"]);
            }
        }
    });
};

function select_links()
{
    var checkboxes = $('form#slot_and_masks_select').find(':checkbox');
    checkboxes.prop('checked', true);
    var masks = $('form#slot_and_masks_select').find(':input[type=text]');
    masks.attr('value','0xFFC');
};

function deselect_links()
{
    var checkboxes = $('form#slot_and_masks_select').find(':checkbox');
    checkboxes.prop('checked', false);
    var masks = $('form#slot_and_masks_select').find(':input[type=text]')
    masks.attr('value','0x000');
};

function select_dacscans()
{
    //$.event.preventDefault();
    var checkboxes = $('form#dacScanV3_select').find(':checkbox');
    checkboxes.prop('checked', true);
};

function deselect_dacscans()
{
    //$.event.preventDefault();
    var checkboxes = $('form#dacScanV3_select').find(':checkbox');
    checkboxes.prop('checked', false);  
};

function default_dacscans()
{
    //$.event.preventDefault();
    var checkboxes = $('form#dacScanV3_select').find(':checkbox');
    checkboxes.prop('checked', true);
    document.getElementById("CFG_CAL_DAC").checked=false;
    document.getElementById("CFG_VREF_ADC").checked=false;
    if (window.jQuery) {
        alert("Selecting all except CFG_CAL_DAC and CFG_VREF_ADC.");
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


