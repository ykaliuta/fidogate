#!/usr/bin/env perl
#use strict;
#
my %config = (checkincludedtext => 0,
              includedcutoff => 40,
              includedratio => 0.6,
              quotere => '^[>:]',
              antiquotere => '^[<]',
             );

sub filter_post {
    my $rval = "" ;             # assume we'll accept.

    if ($config{checkincludedtext}) {
        my ($lines, $quoted, $antiquoted) = analyze($body);
        if ($lines > $config{includedcutoff}
                && $quoted - $antiquoted > $lines * $config{includedratio}) {
            $rval = "Article contains too much quoted text";
        }
    }

    my $grephistory = '/usr/local/news/bin/grephistory';
    my $sm = '/usr/local/news/bin/sm';
    if( $hdr{"Newsgroups"} =~ /fido\./ ) {
        if ( $hdr{"Newsgroups"} =~ /,/ ) {
        $rval = "More than one newsgroup requested, only one allowed"; }
        if (    !(($hdr{"Comment-To"} ne "") ||
                ($hdr{"X-Comment-To"} ne "") ||
                ($hdr{"X-FTN-To"} ne "") ||
                ($hdr{"X-Fidonet-Comment-To"} ne "") ||
                ($hdr{"X-Apparently-To"} ne ""))) {
            my $refs = $hdr{"References"};
            if( $refs ne "" ) {
                my @refs = split(/ /, $refs);
                my $msgid = pop @refs;
                $msgid =~ s/[<>|;\s'"]//g;
                my $fn=`$grephistory \'$msgid\' 2>&1`;
                if( (!($fn =~ /\/dev\/null/)) &&
                    (!($fn =~ /Not found/)) ) {
                    my $orig_from = `$sm $fn`;
                    my @orig_from = grep(/^From:/, split(/\n/, $orig_from));
                    $orig_from = shift @orig_from;
                    $orig_from =~ s/^From:\s*//;
# это должно быть в одну строчку:
                    $orig_from =~
s/^\s*(.+[^\s])\s*<[^\s><"\(\)\@]+\@[^\s><"\(\)\@]+>\s*$/$1/;
# это должно быть в одну строчку:
                    $orig_from =~
s/^\s*<{0,1}[^\s><"\(\)\@]+\@[^\s><"\(\)\@]+>{0,1}\s*\((.+)\)\s*$/$1/;
                    $orig_from =~ s/^"//;
                    $orig_from =~ s/"$//;
                    $hdr{"X-Comment-To"} = $orig_from;
                    $modify_headers = 1;
                    }
                }
        }
    }

    return $rval;
}

sub analyze {
    my ($lines, $quoted, $antiquoted) = (0, 0, 0);
    local $_ = shift;

    do {
        if ( /\G$config{quotere}/mgc ) {
            $quoted++;
        } elsif ( /\G$config{antiquotere}/mgc ) {
            $antiquoted++;
        }
    } while ( /\G(.*)\n/gc && ++$lines );

    return ($lines, $quoted, $antiquoted);
}

