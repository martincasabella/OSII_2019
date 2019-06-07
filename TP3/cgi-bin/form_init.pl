#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';

use CGI qw/:standard/;

my $q = CGI->new;
my $year = $q->param('year');
my $day = $q->param('day');

my @response;
my $c = 0;

print "Content-type: text/html\n\n";

print "<html>\n";
print "<head>";
print "<title>Sistemas Operativos II</title>";
print "<meta name=\"description\" content=\"website description\" /> ";
print "<meta name=\"keywords\" content=\"website keywords, website keywords\" />";
print "<meta http-equiv=\"content-type\" content=\"text/html; charset=windows-1252\" />";
print "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://fonts.googleapis.com/css?family=Tangerine&amp;v1\" />";
print "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://fonts.googleapis.com/css?family=Yanone+Kaffeesatz\"/>";
print "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/style.css\" />";
print "</head>";


print "<body>";
<body bgcolor="#ff69b4" value="F" >;
print "<div id=\"main\">";
print "<div id=\"header\">";
print "<div id=\"logo\">";
print "<h1>Sistemas Operativos II</h1>";
print "</div>";
print "<div id=\"menubar\">";
print "<ul id=\"menu\">";
print "<li><a href=\"http://192.168.0.45/index.html\">Home</a></li>";
print "<li><a href=\"http://192.168.0.45/cgi-bin/system_info.pl\">System Resources</a></li>";
print "<li class=\"current\"><a  href=\"http://192.168.0.45/cgi-bin/form_init.pl\">GOES Info</a></li>";
print "<li><a href=\"cgi-bin/kmod.cgi\">Modules</a></li>";
print "</ul>";
print "</div>";
print "</div>";


print "<div id=\"content\">";
print "<h1>GOES Info</h1>";
print "      <p></p>";
print "   <p></p>";

print  start_html('Form Test');

if ($year && $day) {

    my $raw_out  = qx(aws --no-sign-request s3 ls s3://noaa-goes16/ABI-L2-CMIPF/$year/$day/ | grep M3C13");
    my @lines = split /\n/, $raw_out;
    foreach my $line (@lines) {
        my @entry = split('\s+',$line,4);
        $response[$c] = \@entry;
        print "@response";
        $c++;
    }



} else {
    print start_form;
    print "        <p></p>";
    print "Year: ", textfield('year'), br;
    print "        <p></p>";
    print "        <p></p>";
    print "        <p></p>";
    print "Day: ", textfield('day'), br;
    print "        <p></p>";
   print submit;
       print end_form;
}
print "      <p></p>";
print end_html;
print "      <p></p>";
print "    <p></p>";
print "        <p></p>";

print "</div>";
print "</div>";
print "<p></p>";
print "<p></p>";

print " <div id=\"footer\">";
print "Martin Casabella</p>martin.casabella\@alumnos.unc.edu.ar</p>";
print " </div>";
print " </div>";
print " </body>";
print "  </html>";































