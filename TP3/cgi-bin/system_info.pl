#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';
use lib qw<blib/lib blib/arch>;

use CGI::Carp qw(fatalsToBrowser);
use Linux::SysInfo qw<sysinfo>;
use Proc::CPUUsage;
use Unix::Uptime qw(:hires);







#para uso CPU
my $cp = Proc::CPUUsage->new;
#para sys info
my $si = sysinfo;

my @months = qw( Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec );
my @days = qw(Sun Mon Tue Wed Thu Fri Sat Sun);
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime();
$year += 1900;

#Date/time
my $current_date = "$days[$wday] $mday $months[$mon] $year";
my $current_time = "$hour:$min:$sec";

#CPU usage
my $usage1 = $cp->usage*100; ## returns usage since new()
my $usage2 = $cp->usage*100; ## returns usage since last usage()

#Uptime
my $uptime = Unix::Uptime->uptime(); # 2345
# # "HiRes" mode
my $uptime_sec =Unix::Uptime->uptime_hires();

my ($load1, $load5, $load15) = Unix::Uptime->load(); # (1.0, 2.0, 0.0)










#Info a imprimir





#HTML FORMAT PAGE
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
print "<li><a href=\"../index.html\">Home</a></li>";
print "<li class=\"current\"><a href=\"system_info.pl\">System Resources</a></li>";
print "<li><a href=\"form_init.pl\">GOES Info</a></li>";
print "<li><a href=\"modules.pl\">System modules</a></li>";
print "<li><a href=\"../upload_module.html\">Install module</a></li>";
print "</ul>";
print "</div>";
print "</div>";


print "<div id=\"content\">";
print "<h1>System Resources</h1>";
print "<p>Date: $current_date        Time: $current_time";
print "<p><p>System information:</p>";
print "<p>- $_: $si->{$_}" for keys %$si;
print "<p>- CPU usage (last measurement):  $usage2 %</p>";
print "<p> -Uptime: " , $uptime_sec, " [s]</p>";
print "<p> -Load average for last minute: " ,$load1, </p>;

print "     <p></p>";
print "  <p></p>";
print "      <p></p>";
print "   <p></p>";
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

