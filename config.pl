##############################################################################
#
#
# Perl functions to read FIDOGATE config file,
# included by <INCLUDE config.pl> when running subst.pl
#

my %CONFIG;

# specials for DosDrive and Zone
my %CONFIG_dosdrive;
my %CONFIG_zone;



my %CONFIG_default =
    (
     <GEN perl-default>
     );
my %CONFIG_abbrev =
    (
     <GEN perl-abbrev>
     );



sub CONFIG_read {
    my($file) = @_;
    my($key, $arg);
    local *C;

    $file = CONFIG_expand($file);

    open(C,"$file") || die "config.pl: can't open config file $file\n";
    while(<C>) {
	chop;
	next if( /^\s*\#/ );	# comments
	next if( /^\s*$/  );	# empty
	s/\s*$//;		# remove trailing white space
	s/^\s*//;		# remove leading white space
	($key,$arg) = split(' ', $_, 2);
	$key =~ tr/A-Z/a-z/;
	if($key eq "include") {
	    CONFIG_read($arg);
	    next;
	}
	if($key eq "dosdrive") {
	    my ($d, $path) = split(' ', $arg);
	    $CONFIG_dosdrive{lc($d)} = $path;
	    next;
	}
	if($key eq "zone") {
	    my ($z, $rest) = split(' ', $arg, 2);
	    $CONFIG_zone{$z} = $rest;
	    next;
	}
	$CONFIG{$key} = $arg if(!$CONFIG{$key});
    }
    close(C);
}


sub CONFIG_get1 {
    my($key) = @_;
    my($ukey);

    $ukey = $key;
    $ukey =~ tr/a-z/A-Z/;
    return $ENV{"FIDOGATE_$ukey"} if($ENV{"FIDOGATE_$ukey"});

    return $CONFIG{$key} if($CONFIG{$key});
    return $CONFIG_default{$key};
}


sub CONFIG_get {
    my($key) = @_;
    my($ret);
    my($exp);

    $key =~ tr/A-Z/a-z/;
    return CONFIG_expand( CONFIG_get1($key) );
}


sub CONFIG_expand {
    my($v) = @_;
    my($exp);

    if($v =~ /^%([A-Z])/) {
	$exp = CONFIG_get1($CONFIG_abbrev{$1});
	$v =~ s/^%./$exp/;
    }

    return $v;
}


sub CONFIG_debug {    
    my($key);

    for $key (keys %CONFIG) {
	print "$key = $CONFIG{$key} -> ", CONFIG_get($key), "\n";
    }
}

##############################################################################
