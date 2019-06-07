#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';

use CGI qw(:standard);
use CGI::Carp qw(fatalsToBrowser);
use JSON;


my $query = new CGI;

my @response;
my $c = 0;

my $raw_out  = qx(aws --no-sign-request s3 ls s3://noaa-goes16/ABI-L2-CMIPF/2019/007/ 2>&1);

my @lines = split /\n/, $raw_out;

foreach my $line (@lines) {
    my @entry = split('\s+',$line,4);
    $response[$c] = \@entry;
    print $response[$c];
    $c++;
}

my $jsonResponse = encode_json \@response;
