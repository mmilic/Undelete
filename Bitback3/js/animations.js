
$(function() {
  $('a[href*=#]:not([href=#])').click(function() {
    if (location.pathname.replace(/^\//,'') == this.pathname.replace(/^\//,'') && location.hostname == this.hostname) {
      var target = $(this.hash);
      target = target.length ? target : $('[name=' + this.hash.slice(1) +']');
      if (target.length) {
        $('html,body').animate({
          scrollTop: target.offset().top
        }, 1000);
        return false;
      }
    }
  });
});



        $(document).ready(function(){
            $(window).bind('scroll', function() {
              var navHeight = $( window ).height() - 10;
                if ($(window).scrollTop() > navHeight) {
                    $('nav').addClass('fixed');
                }
                else {
                    $('nav').removeClass('fixed');
                }
            });
        });



        $(document).ready(function(){
            $(window).bind('scroll', function() {
              var navHeight = $( window ).height() - 10;
                if ($(window).scrollTop() > navHeight) {
                    $('.logo').addClass('fixedlogo');
                }
                else {
                    $('.logo').removeClass('fixedlogo');
                }
            });
        });



        $(document).ready(function(){
            $(window).bind('scroll', function() {
              var navHeight = $( window ).height() - 10;
                if ($(window).scrollTop() > navHeight) {
                    $('.whitespace').addClass('fixedwhite');
                }
                else {
                    $('.whitespace').removeClass('fixedwhite');
                }
            });
        });
