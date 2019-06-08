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
print "<li><a href=\"../goes.html\">GOES Info</a></li>";
print "<li><a href=\"modules.pl\">System modules</a></li>";
print "<li class=\"current\" ><a href=\"../upload_module.html\">Install module</a></li>";
print "</ul>";
print "</div>";
print "</div>";


print "<div id=\"content\">";
print "<h1>Install or remove kernel module</h1>";



$CGI::POST_MAX = 1024 * 5000;

# my $safe_filename_characters = "a-zA-Z0-9_.-";
my $upload_dir = "/srv/http/uploads";
my $query = new CGI;
my $filename = $query->param("module");

#COn el parametro module recibido (y archivo) procedo a validarlo

if ( !$filename ) {
    # print $query->header ( );
    # print "There was a problem uploading your photo (try a smaller file).";
    print "<p>Iput not interpreted\n\n";
    exit;
}


#Parseo la extension
my ( $name, $path, $extension ) = fileparse ( $filename, '..*' );
$filename = $name . $extension;

$filename =~ tr/ /_/;
# $filename =~ s/[^$safe_filename_characters]//g;

if ($filename =~ /^(\w+[\w.-]+\.ko)$/o) {
    $filename = $1;
    print "<p> Checked file: [ OK ]. <p>"
}
else {
    die "<p> Not a module file<p>";
}

my $upload_filehandle = $query->upload("module");

open ( UPLOADFILE, ">$upload_dir/$filename" ) or die "$!";
binmode UPLOADFILE;

while ( <$upload_filehandle> ){
    print UPLOADFILE;
}

close UPLOADFILE;


chdir "../uploads";
print `/usr/bin/sudo /usr/bin/insmod $filename`;
print " Module uploaded to server and installed correctly <p>.";

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


















































