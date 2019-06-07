#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';
use CGI;
use CGI::Carp qw ( fatalsToBrowser );
use File::Basename;
use File::chdir;




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
print "<li ><a href=\"system_info.pl\">System Resources</a></li>";
print "<li><a href=\"form_init.pl\">GOES Info</a></li>";
print "<li><a href=\"modules.pl\">System modules</a></li>";
print "<li class=\"current\" ><a href=\"../upload_module.html\">Install module</a></li>";
print "</ul>";
print "</div>";
print "</div>";


print "<div id=\"content\">";
print "<h1>Install or remove kernel module</h1>";
print "<p> Module removed <p>";

chdir "../uploads";
my $dirname = "/srv/http/uploads";
opendir ( DIR, $dirname ) || die "Error in opening dir $dirname\n";
my @files = grep(/\.ko$/,readdir(DIR));
closedir(DIR);
foreach my $file (@files) {
      print `/usr/bin/sudo /usr/bin/rmmod $file`;
      print `/usr/bin/sudo /usr/bin/rm $file`;
}




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












