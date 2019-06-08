#!/usr/bin/perl -w
use warnings;
use strict;
use CGI qw(:standard);
use CGI::Carp qw(fatalsToBrowser);
use File::Basename;
use File::chdir;
#Instancia CGI
my $query = new CGI;


#obtengo fecha como parametro del ususario
my $date = $query->param("date");
#my $date = "2019/025/01";
my @words = split '/', $date;

my $year = $words[0];
my $day = sprintf '%03s', $words[1];
my $hour = sprintf '%02s', $words[2];


#Ejecuto comando aws, y parseo su salida, para asi imprimirla
my $raw_out  = qx(/usr/bin/aws --no-sign-request s3 ls s3://noaa-goes16/ABI-L2-CMIPF/$year/$day/$hour/ 2>&1);

#print `/usr/bin/aws --no-sign-request s3 ls s3://noaa-goes16/ABI-L2-CMIPF/2019/001/23/ 2>&1`;
my @lines = split /\n/, $raw_out;


#Muestro pagina html para seguir formato de las demas
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
print "<li class=\"current\" ><a href=\"../goes.html\">GOES Info</a></li>";
print "<li><a href=\"modules.pl\">System modules</a></li>";
print "<li><a href=\"../upload_module.html\">Install module</a></li>";
print "</ul>";
print "</div>";
print "</div>";


print "<div id=\"content\">";
print "<h1>GOES 16 archives</h1>";

print "$year\n";
print "$day\n";
print "$hour\n";
print"<p><\p>";

# Imprimo lista de archivos en el html para que el usuario vea lo solicitado
foreach my $line (@lines) {
    my @entry = split('\s+',$line,4);
    print "$entry[0]      $entry[1]   $entry[2]       $entry[3] ";


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


