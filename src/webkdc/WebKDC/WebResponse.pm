package WebKDC::WebResponse;

use strict;
use warnings;

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    # set the version for version checking
    $VERSION     = 1.00;
    @ISA         = qw(Exporter);
    @EXPORT      = qw();
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],

    # your exported package globals go here,
    # as well as any optionally exported functions
    @EXPORT_OK   = qw();
}

our @EXPORT_OK;

sub new {
    my $type = shift;
    my $self = {};
    bless $self, $type;
    return $self;
}

sub return_url {
    my $self = shift;
    $self->{'return_url'} = shift if @_;
    return $self->{'return_url'};
}

sub post_url {
    my $self = shift;
    $self->{'post_url'} = shift if @_;
    return $self->{'post_url'};
}

sub proxy_cookie {
    my $self = shift;
    my $type = shift;
    $self->{'cookies'}{"webauth_pt_$type"} = shift if @_;
    return $self->{'cookies'}{"webauth_pt_$type"};
}

sub proxy_cookies {
    my $self = shift;
    return $self->{'cookies'};
}

sub response_token {
    my $self = shift;
    $self->{'response_token'} = shift if @_;
    return $self->{'response_token'};
}


sub app_state_token {
    my $self = shift;
    $self->{'app_state_token'} = shift if @_;
    return $self->{'app_state_token'};
}

1;
