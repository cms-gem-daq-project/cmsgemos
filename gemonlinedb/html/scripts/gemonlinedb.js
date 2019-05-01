// How often the data is updated (ms).
const UPDATE_INTERVAL = 5000;

/*
 * Displays an error message when a JSON XHR fails.
 */
var showError = function(data, textStatus, error)
{
    console.error(
        "Failed to update JSON data, status: " + textStatus + ", error: "+error);
}

/*
 * Updates the monitoring data.
 */
var updateMonitoring = function(json)
{
    $("#topology-source-mode").text(json.topology.mode);
    var sourceDetails = $("#topology-source-details");
    sourceDetails.empty();
    for (var source of json.topology.sources) {
        sourceDetails.append("<li>" + source);
    }

    $("#objects-source-mode").text(json.objects.mode);
    var sourceDetails = $("#objects-source-details");
    sourceDetails.empty();
    for (var source of json.objects.sources) {
        sourceDetails.append("<li>" + source);
    }

    $("#amc13-count").text(json.statistics["amc13-count"]);
    $("#amc-count").text(json.statistics["amc-count"]);
    $("#oh-count").text(json.statistics["oh-count"]);
    $("#vfat-count").text(json.statistics["vfat-count"]);
}

/*
 * Starts updating the displayed data.
 */
function startUpdate(jsonURL)
{
    var updateFunction = function()
    {
        $.getJSON(jsonURL).done(updateMonitoring).fail(showError);
    }
    updateFunction(); // Update immediately
    setInterval(updateFunction, UPDATE_INTERVAL);
}
