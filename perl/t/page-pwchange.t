#!/usr/bin/perl -w
#
# page-pwchange.t - Tests for proper output when the pwchange page is printed
#
# Written by Jon Robertson <jonrober@stanford.edu>
# Copyright 2010 Board of Trustees, Leland Stanford Jr. University
#
# See LICENSE for licensing terms.

use strict;
use warnings;

use lib ('t/lib', 'lib', 'blib/arch');

use WebLogin;
use CGI;

use Test::More tests => 44;

# Load a version of the page templates that just prints out the vars sent.
my %pages = (pwchange => 'pwchange.tmpl');
%pages = map {
    $_    => HTML::Template->new (filename => $pages{$_},
    cache => 1,
    path  => 't/data/templates')
} keys %pages;

# Set up a query with some test data.
my $query = CGI->new;
$query->param ('username', 'testuser');
$query->param ('expired', 1);

# Fake a weblogin object.
my $weblogin = {};
bless $weblogin, 'WebLogin';
$weblogin->{query} = $query;
$weblogin->{pages} = \%pages;

# Move stdout to a string so we can check the page output.
my ($oldout, $page);
open (PAGE, '>', \$page) or die "could not open string for writing";
select PAGE;
WebLogin::print_pwchange_page ($weblogin, 'TestRT', 'TestST');
select STDOUT;
close PAGE;
my @output = split (/[\r\n]+/, $page);

# Check to make sure the page printed as we expected.
my $line;
do {
    $line = shift @output;
} until $line =~ /Content-Type: /;
ok ($page, 'pwchange page was printed');
is ($output[0], 'error ', ' and error was not set');
is ($output[1], 'err_username ', ' and err_username was not set');
is ($output[2], 'err_password ', ' and err_password was not set');
is ($output[3], 'err_newpassword ', ' and err_newpassword was not set');
is ($output[4], 'err_newpassword_match ',
    ' and err_newpassword_match was not set');
is ($output[5], 'err_loginfailed ', ' and err_loginfailed was not set');
is ($output[6], 'err_rejected ', ' and err_rejected was not set');
is ($output[7], 'err_pwweak ', ' and err_pwweak was not set');
is ($output[8], 'err_pwchange ', ' and err_pwchange was not set');
is ($output[9], 'err_msg ', ' and err_msg was not set');
is ($output[10], 'RT TestRT', ' and RT was set');
is ($output[11], 'ST TestST', ' and ST was set');
is ($output[12], 'CPT ', ' and CPT was not set');
is ($output[13], 'username testuser', ' and username was set');
is ($output[14], 'password ', ' and password was not set');
is ($output[15], 'new_passwd1 ', ' and new_passwd1 was not set');
is ($output[16], 'new_passwd2 ', ' and new_passwd2 was not set');
is ($output[17], 'changepw ', ' and changepw was not set');
is ($output[18], 'expired 1', ' and expired was set');
is ($output[19], 'skip_username ', ' and skip_username was not set');
is ($output[20], 'skip_password ', ' and skip_password was not set');

# Once more, testing CPT suppressing the username and password.
$weblogin->{CPT} = 'TestCPT';
open (PAGE, '>', \$page) or die "could not open string for writing";
select PAGE;
WebLogin::print_pwchange_page ($weblogin, 'TestRT2', 'TestST2');
select STDOUT;
close PAGE;
@output = split (/[\r\n]+/, $page);
do {
    $line = shift @output;
} until $line =~ /Content-Type: /;

ok ($page, 'pwchange page was printed with CPT');
is ($output[0], 'error ', ' and error was not set');
is ($output[1], 'err_username ', ' and err_username was not set');
is ($output[2], 'err_password ', ' and err_password was not set');
is ($output[3], 'err_newpassword ', ' and err_newpassword was not set');
is ($output[4], 'err_newpassword_match ',
    ' and err_newpassword_match was not set');
is ($output[5], 'err_loginfailed ', ' and err_loginfailed was not set');
is ($output[6], 'err_rejected ', ' and err_rejected was not set');
is ($output[7], 'err_pwweak ', ' and err_pwweak was not set');
is ($output[8], 'err_pwchange ', ' and err_pwchange was not set');
is ($output[9], 'err_msg ', ' and err_msg was not set');
is ($output[10], 'RT TestRT2', ' and RT was set');
is ($output[11], 'ST TestST2', ' and ST was set');
is ($output[12], 'CPT TestCPT', ' and CPT was set');
is ($output[13], 'username testuser', ' and username was set');
is ($output[14], 'password ', ' and password was not set');
is ($output[15], 'new_passwd1 ', ' and new_passwd1 was not set');
is ($output[16], 'new_passwd2 ', ' and new_passwd2 was not set');
is ($output[17], 'changepw ', ' and changepw was not set');
is ($output[18], 'expired 1', ' and expired was set');
is ($output[19], 'skip_username 1', ' and skip_username was set');
is ($output[20], 'skip_password 1', ' and skip_password was set');