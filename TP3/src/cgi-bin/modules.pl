#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';
use lib qw<blib/lib blib/arch>;
use CGI qw(:standard);


#Obtengo lista de modulos
my $out = qx(lsmod 2>&1);
#my @lines = split /\n/, $out; #guardo en array lines, lista partida en \n


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
;

print "<li><a href=\"../index.html\">Home</a></li>";
print "<li><a href=\"system_info.pl\">System Resources</a></li>";
print "<li><a href=\"../goes.html\">GOES Info</a></li>";
print "<li class=\"current\"><a href=\"modules.pl\">System modules</a></li>";
print "<li><a href=\"../upload_module.html\">Install module</a></li>";
print "</ul>";
print "</div>";
print "</div>";

print "<div id=\"content\">";
print "<h1>Installed system modules</h1>";


#Parseo salida lsmod y la muestro en pantalla
my @lines = split /\n/, $out;

#Ssimplemente para obviar la primera linea de salida
my $first = 1;
foreach my $line (@lines) {
    my @entry = split('\s+',$line,3);

    if ($first==1){
        $first = 0;
    }else{
        print "<p> Module:   $entry[0]  &nbsp;  Size:   $entry[1]  &nbsp; Used by:   $entry[2] ";
    }
}


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

