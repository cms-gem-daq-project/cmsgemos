$(document).ready(function() {
        gemFooterStick();
    });

function gemFooterStick()
{
    console.log("gemFooterStick");
    // Store the main divs in local variables for convienience
    var content = $('#xdaq-main');
    var footer = $('.gem-footer');

    //////////
    // Footer position
    //////////

    // Set the height to auto to force an auto height calculation (or the page will not be able to decrease in size)
    content.css({'height': 'auto'});

    // Height of the user content area
    contentHeight = content.outerHeight();

    // Combined height of the header and footer
    footHeight = footer.outerHeight();

    // if content doesnt need scrolling, force content height to be full height
    if (contentHeight < $(window).height() - footHeight)
        {
            content.css({'height': $(window).height() - footHeight});
        }

    //////////
    //  Footer width
    //////////

    // make sure footer is the right size allowing for user content to be wider than the viewport

    footer.css({'width': 'auto'});

    // viewport width
    var minWidth = $(window).width();

    // if the user content plus the sidebar (inc padding) is wider than the viewport, use that width
    var wrapper = content.parent().width();
    if (minWidth < wrapper)
        {
            minWidth = wrapper;
        }

    // Set the css
    footer.css({'width': minWidth});
}
