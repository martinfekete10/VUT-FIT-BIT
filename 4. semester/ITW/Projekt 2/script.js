/*
* Author: Martin Fekete <xfeket00@fit.vutbr.cz>
* Accessible at: http://martinfekete.aspfree.cz/
*/

// Responsive menu
function responsiveMenu() {
    var x = document.getElementById("menu");
    
    if (x.className === "main-menu") {
        x.className += " responsive";
    } else {
        x.className = "main-menu";
    }
}

// Remove "active" class every menu element except element skip
function removeAcitve(skip) {
    for (var i = 0; i < 6; i++) {
        if (i == skip) {
            continue;
        }
        $("#".concat(i)).removeClass("active");
    }
}

// Add "active" class to menu element
$(window).scroll(function() {
    var scroll = $(window).scrollTop();
    // Home
    if (scroll < 500) {
        removeAcitve(1);
        $("#1").addClass("active");
    // About Me
    } else if (scroll >= 500 && scroll < 1350) {
        removeAcitve(2);
        $("#2").addClass("active");
    // Technologies
    } else if (scroll >= 1350 && scroll < 2050) {
        removeAcitve(3);
        $("#3").addClass("active");
    // Portfolio
    } else if (scroll >= 2050 && scroll < 2800) {
        removeAcitve(4);
        $("#4").addClass("active");
    // Contact Me
    } else {
        removeAcitve(5);
        $("#5").addClass("active");
    }
});

// Check if user reached bottom of page
// Make menu element "Contact" active if yes
$(window).scroll(function() {
    if($(window).scrollTop() + $(window).height() == $(document).height()) {
        removeAcitve(5);
        $("#5").addClass("active");
    }
});