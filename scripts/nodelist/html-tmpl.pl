#
# $Id: html-tmpl.pl,v 4.1 1997/06/22 10:25:19 mj Exp $
#
# Perl routines for doing substitutions in HTML template
#



#
# Read input HTML files, substitute %ABC% strings, write to STDOUT
#
# Substitutions:  %MAIN% (entire line)  is replaced with a call to &HtmlMain
#                 %X% (upper case only) is replaced with $HtmlSubst{'X'}
#

sub HtmlTemplate {
    local($tmpl) = @_;

    open(TMPL, "$tmpl") || &CgiDie("CGI script can't open $tmpl");

    while(<TMPL>) {
	# Call HtmlMain() for %MAIN%
	if( /^\s*%MAIN%\s*$/ ) {
	    &HtmlMain();
	    next;
	}

	# Substitutions
	s/%([A-Z]+)%/$HtmlSubst{$1}/g;

	# Output
	print;
    }
    
    close(TMPL);
}



#
# HTML-quotes string
#

sub HtmlQuote {
    local($s) = @_;

    $s =~ s/\</&lt;/g;
    $s =~ s/\>/&gt;/g;
    $s =~ s/\&/&amp;/g;

    return $s;
}



#
# Date
#

sub HtmlDate {
    local @month_names =
	("Jan", "Feb", "Mar", "Apr", "May", "Jun",
	 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec");

    ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime;
    
    return sprintf "%02d %s %s", $mday, $month_names[$mon], $year+1900;
}



1; #return true 
