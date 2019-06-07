#!/usr/bin/perl -w
use warnings;
use strict;
use CGI qw(:standard);
use CGI::Carp qw(fatalsToBrowser);
use JSON;

#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';

use CGI qw/:standard/;

my $q = CGI->new;
my $year = $q->param('year');
my $day = $q->param('day');

print header, start_html('Form Test');

print start_form,
    'Enter date, br,
    'Year: ', textfield('year'), br,
        submit, end_form;
}
print end_html;