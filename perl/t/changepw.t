#!/usr/bin/perl -w
#
# changepw.t - Test the password change functions in WebLogin module.
#
# Written by Jon Robertson <jonrober@stanford.edu>
# Copyright 2010 Board of Trustees, Leland Stanford Jr. University
#
# See LICENSE for licensing terms.

use strict;
use warnings;

use lib ('t/lib', 'lib', 'blib/arch');
use Util qw (get_userinfo create_keyring);

use CGI;
use HTML::Template;

use WebAuth qw(:base64 :const :krb5 :key);
use WebLogin;
use WebKDC ();

use Test::More;

# Whether we've found a valid kerberos config.
my $kerberos_config = 0;

# Get the username we need to change, and its current password.
my $fname_passwd = 't/data/test.password';
my ($username, $password) = get_userinfo ($fname_passwd) if -f $fname_passwd;
if ($username && $password) {
    $kerberos_config = 1;
}

if ($kerberos_config) {
    plan tests => 7;
} else {
    plan skip_all => 'no kerberos configuration found';
}

# New password to try changing the user to.
# FIXME: Should we use apg to generate each time?  Adds a testing requirement.
my $newpassword = 'dujPifecvij3';

# Set up a query with some test data.
my $query = new CGI;
my %pages = ();
my $weblogin = new WebLogin ($query, \%pages);
$weblogin->{logging} = 0;

# Create the keyring to use.
$WebKDC::Config::KEYRING_PATH = 't/data/test.keyring';
create_keyring ($WebKDC::Config::KEYRING_PATH);

# Test a successful password change.
$weblogin->{query}->param ('username', $username);
$weblogin->{query}->param ('password', $password);
$weblogin->{query}->param ('new_passwd1', $newpassword);
$weblogin->add_changepw_token;
my ($status, $error) = $weblogin->change_user_password;
is ($status, WebKDC::WK_SUCCESS, 'changing the password works');
print "Status: $status\nError: $error\n";

# And undo it.
$weblogin->{query}->param ('password', $newpassword);
$weblogin->{query}->param ('new_passwd1', $password);
($status, $error) = $weblogin->change_user_password;
is ($status, WebKDC::WK_SUCCESS, ' as does changing it back');

# Test going to change_user_password with password but not CPT (should work)
$weblogin->{CPT} = '';
$weblogin->{query} = new CGI;
$weblogin->{query}->param ('username', $username);
$weblogin->{query}->param ('password', $password);
$weblogin->{query}->param ('new_passwd1', $newpassword);
($status, $error) = $weblogin->change_user_password;
is ($status, WebKDC::WK_SUCCESS,
    'changing the password with old password but no CPT works');

# And undo it.
$weblogin->{query}->param ('password', $newpassword);
$weblogin->{query}->param ('new_passwd1', $password);
($status, $error) = $weblogin->change_user_password;
is ($status, WebKDC::WK_SUCCESS, ' as does changing it back');

# Test going to change_user_password no CPT or password (should not work).
$weblogin->{query} = new CGI;
$weblogin->{query}->param ('username', $username);
$weblogin->{query}->param ('new_passwd1', $newpassword);
$weblogin->{CPT} = '';
($status, $error) = $weblogin->change_user_password;
isnt ($status, WebKDC::WK_SUCCESS,
      'changing the password without password or CPT fails');

# Test trying a simple password 'abc' (should not work)
# FIXME: Test exact error code, not isn't.  Allow success or failure if it's
# not strong enough password (and if success, change the password back).
$weblogin->{query} = new CGI;
$weblogin->{query}->param ('username', $username);
$weblogin->{query}->param ('password', $password);
$weblogin->{query}->param ('new_passwd1', 'cat');
$weblogin->add_changepw_token;
($status, $error) = $weblogin->change_user_password;
isnt ($status, WebKDC::WK_SUCCESS,
    'changing the password to dictionary word fails');

# Test creating CPT, then sending different username to change_user_password
# (should not work)
$weblogin->{query} = new CGI;
$weblogin->{query}->param ('username', $username);
$weblogin->{query}->param ('password', $password);
$weblogin->{query}->param ('new_passwd1', $newpassword);
$weblogin->add_changepw_token;
$weblogin->{query}->param ('username', $username.'_doe');
($status, $error) = $weblogin->change_user_password;
isnt ($status, WebKDC::WK_SUCCESS, 'changing the password of a user fails');

# Clean up the keyring.
unlink ($WebKDC::Config::KEYRING_PATH);
