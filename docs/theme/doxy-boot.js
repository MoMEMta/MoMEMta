hljs.configure({
  tabReplace: '    ',
  languages: ['cpp']
});

$(document).ready(function() {

    $(".fragment").hide();

    $("div.headertitle").addClass("page-header");
    $("div.title").addClass("h1");

    $('li > a[href="index.html"] > span').before("<i class='fa fa-cog'></i> ");
    $('li > a[href="modules.html"] > span').before("<i class='fa fa-square'></i> ");
    $('li > a[href="namespaces.html"] > span').before("<i class='fa fa-bars'></i> ");
    $('li > a[href="annotated.html"] > span').before("<i class='fa fa-list-ul'></i> ");
    $('li > a[href="classes.html"] > span').before("<i class='fa fa-book'></i> ");
    $('li > a[href="inherits.html"] > span').before("<i class='fa fa-sitemap'></i> ");
    $('li > a[href="functions.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="functions_func.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="functions_vars.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="functions_enum.html"] > span').before("<i class='fa fa-list'></i> ");
    $('li > a[href="functions_eval.html"] > span').before("<i class='fa fa-list'></i> ");
    $('img[src="ftv2ns.png"]').replaceWith('<span class="label label-primary">N</span> ');
    $('img[src="ftv2cl.png"]').replaceWith('<span class="label label-primary">C</span> ');

    var main_nav_ul = $("div#navrow1 > ul.tablist");
    $("nav.navbar > div.container").append(main_nav_ul);
    main_nav_ul.addClass("nav navbar-nav")
        .removeClass("tablist")
        .wrap("<div class='collapse navbar-collapse' id='main-navbar'><div>");

    var main_nav_bar = $('#main-navbar');

    $("ul.tablist").addClass("nav nav-pills");
    $("ul.tablist").css("margin-top", "0.5em");
    $("ul.tablist").css("margin-bottom", "0.5em");
    $("li.current").addClass("active");

    $("iframe").attr("scrolling", "yes");

    $("#nav-path > ul").addClass("breadcrumb");

    $("table.params").addClass("table");
    $("div.ingroups").wrapInner("<small></small>");
    $("div.levels").css("margin", "0.5em");
    $("div.levels > span").addClass("btn btn-default btn-xs");
    $("div.levels > span").css("margin-right", "0.25em");

    $("table.directory").addClass("table table-striped");
    $("div.summary > a").addClass("btn btn-default btn-xs");
    $("table.fieldtable").addClass("table");
    //$(".fragment").addClass("well");
    $(".memitem").addClass("panel panel-default");
    $(".memproto").addClass("panel-heading");
    $(".memdoc").addClass("panel-body");
    $("span.mlabel").addClass("label label-info");

    $("table.memberdecls").addClass("table");
    $("[class^=memitem]").addClass("active");

    $("div.ah").addClass("btn btn-default");
    $("span.mlabels").addClass("pull-right");
    $("table.mlabels").css("width", "100%");
    $("td.mlabels-right").addClass("pull-right");

    $("div.ttc").addClass("panel panel-primary");
    $("div.ttname").addClass("panel-heading");
    $("div.ttname a").css("color", 'white');
    $("div.ttdef,div.ttdoc,div.ttdeci").addClass("panel-body");

    //$('div.fragment.well div.line:first').css('margin-top', '15px');
    //$('div.fragment.well div.line:last').css('margin-bottom', '15px');

    $('table.doxtable').removeClass('doxtable').addClass('table table-striped table-bordered').each(function() {
        $(this).prepend('<thead></thead>');
        $(this).find('tbody > tr:first').prependTo($(this).find('thead'));

        $(this).find('td > span.success').parent().addClass('success');
        $(this).find('td > span.warning').parent().addClass('warning');
        $(this).find('td > span.danger').parent().addClass('danger');
    });

    //if ($('div.fragment.well div.ttc').length > 0) {
        //$('div.fragment.well div.line:first').parent().removeClass('fragment well');
    //}

    // $('table.memberdecls').find('.memItemRight').each(function(){
    //   $(this).contents().appendTo($(this).siblings('.memItemLeft'));
    //   $(this).siblings('.memItemLeft').attr('align', 'left');
    // });

    function getOriginalWidthOfImg(img_element) {
        var t = new Image();
        t.src = (img_element.getAttribute ? img_element.getAttribute("src") : false) || img_element.src;
        return t.width;
    }

    $('div.dyncontent').find('img').each(function() {
        if (getOriginalWidthOfImg($(this)[0]) > $('#content>div.container').width())
            $(this).css('width', '100%');
    });


    // responsive search box

    $('#MSearchBox').parent().remove();

    var search_box = $('<div class="navbar-form navbar-right" id="search-box" role="search"></div>')
        .append('\
      <div class="form-group">\
        <div class="input-group">\
          <div class="input-group-btn">\
            <button aria-expanded="false" type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown">\
              <span class="glyphicon glyphicon-search"></span> <span class="caret"></span>\
            </button>\
            <ul class="dropdown-menu">\
            </ul>\
          </div> <!-- /btn-group -->\
          <button id="search-close" type="button" class="close" aria-label="Close"><span aria-hidden="true">&times;</span></button>\
          <input id="search-field" class="form-control" accesskey="S" onkeydown="searchBox.OnSearchFieldChange(event);" placeholder="Search ..." type="text">\
        </div>\
      </div>');
    main_nav_bar.append(search_box);

    $('#MSearchSelectWindow .SelectionMark').remove();
    var search_selectors = $('#MSearchSelectWindow .SelectItem');
    for (var i = 0; i < search_selectors.length; i += 1) {
        var element_a = $('<a href="#"></a>').text($(search_selectors[i]).text());

        element_a.click(function() {
            $('#search-box .dropdown-menu li').removeClass('active');
            $(this).parent().addClass('active');
            searchBox.OnSelectItem($('#search-box li a').index(this));
            searchBox.Search();
            return false;
        });

        var element = $('<li></li>').append(element_a);
        $('#search-box .dropdown-menu').append(element);
    }
    $('#MSearchSelectWindow').remove();

    $('#search-box .close').click(function() {
        searchBox.CloseResultsWindow();
    });

    $('body').append('<div id="MSearchClose"></div>');
    $('body').append('<div id="MSearchBox"></div>');
    $('body').append('<div id="MSearchSelectWindow"></div>');

    searchBox.searchLabel = '';
    searchBox.DOMSearchField = function() {
        return document.getElementById("search-field");
    }

    searchBox.DOMSearchClose = function() {
        return document.getElementById("search-close");
    }


    // search results
    var results_iframe = $('#MSearchResults').detach();
    $('#MSearchResultsWindow')
        .attr('id', 'search-results-window')
        .addClass('panel panel-default')
        .append(
            '<div class="panel-heading">\
            <h3 class="panel-title">Search Results</h3>\
          </div>\
          <div class="panel-body"></div>'
        );
    $('#search-results-window .panel-body').append(results_iframe);

    searchBox.DOMPopupSearchResultsWindow = function() {
        return document.getElementById("search-results-window");
    }

    function update_search_results_window() {
        $('#search-results-window').removeClass('panel-default panel-success panel-warning panel-danger');
        var status = $('#MSearchResults').contents().find('.SRStatus:visible');
        if (status.length > 0) {
            switch (status.attr('id')) {
                case 'Loading':
                case 'Searching':
                    $('#search-results-window').addClass('panel-warning');
                    break;
                case 'NoMatches':
                    $('#search-results-window').addClass('panel-danger');
                    break;
                default:
                    $('#search-results-window').addClass('panel-default');
            }
        } else {
            $('#search-results-window').addClass('panel-success');
        }
    }


    $('#MSearchResults').load(function() {
        $('#MSearchResults').contents().find('link[href="search.css"]').attr('href', '../doxygen.css');
        $('#MSearchResults').contents().find('head').append(
            '<link href="../customdoxygen.css" rel="stylesheet" type="text/css">');

        update_search_results_window();

        // detect status changes (only for search with external search
        // backend)
        var observer = new MutationObserver(function(mutations) {
            update_search_results_window();
        });
        var config = {
            attributes: true
        };

        var targets = $('#MSearchResults').contents().find('.SRStatus');
        for (i = 0; i < targets.length; i++) {
            observer.observe(targets[i], config);
        }
    });


    // enumerations
    $('table.fieldtable')
        .removeClass('fieldtable')
        .addClass('table table-striped table-bordered')
        .each(function() {
            $(this).prepend('<thead></thead>');
            $(this).find('tbody > tr:first').prependTo($(this).find('thead'));

            $(this).find('td > span.success').parent().addClass('success');
            $(this).find('td > span.warning').parent().addClass('warning');
            $(this).find('td > span.danger').parent().addClass('danger');
        });

    // todo list
    var todoelements = $(
        '.contents > .textblock > dl.reflist > dt, .contents > .textblock > dl.reflist > dd');
    for (var i = 0; i < todoelements.length; i += 2) {
        $('.contents > .textblock')
            .append(
                '<div class="panel panel-default active">' +
                "<div class=\"panel-heading todoname\">" +
                $(todoelements[i]).html() + "</div>" +
                "<div class=\"panel-body\">" + $(todoelements[i + 1]).html() +
                "</div>" + '</div>');
    }
    $('.contents > .textblock > dl').remove();


    $(".memitem").removeClass('memitem');
    $(".memproto").removeClass('memproto');
    $(".memdoc").removeClass('memdoc');
    $("span.mlabel").removeClass('mlabel');
    $("table.memberdecls").removeClass('memberdecls');
    $("[class^=memitem]").removeClass('memitem');
    $("span.mlabels").removeClass('mlabels');
    $("table.mlabels").removeClass('mlabels');
    $("td.mlabels-right").removeClass('mlabels-right');
    $(".navpath").removeClass('navpath');
    $("li.navelem").removeClass('navelem');
    $("a.el").removeClass('el');
    $("div.ah").removeClass('ah');
    $("div.header").removeClass("header");

    $(".fragment").each(function(i, node) {
        // Remove line numbers
        lines = $(node).find('.lineno').remove();
        have_lines = lines.length != 0;

        // Build code
        code = ''
        $(node).find('.line').each(function(i, line) {
          code += $(line).text() + '\n';
        });
        $(node).html("<pre class='code'><code></code></pre>");
        $(node).find('code').text(code);

        $(node).removeClass('fragment').addClass('highlighted-code');
        if (have_lines) {
            $(node).addClass('line-number');
        }
    });

    $('code').each(function(i, block) {
        hljs.highlightBlock(block);
    });

    function pad(str, max) {
        str = str.toString();
        return str.length < max ? pad("0" + str, max) : str;
    }

    $('div.line-number > pre > code').each(function(i, node) {

        // Re-add line numbers
        code = $(node).html().split('\n');
        flat_code = '';
        for (index = 0; index < code.length; ++index) {
            flat_code += '<a name="l' + pad(index + 1, 5) + '" href="#"></a><span class="line"></span>' + code[index] + '\n';
        }

        $(node).html(flat_code);
    });

    $('.highlighted-code').show();
});
