#!/usr/bin/perl
# ****************************************************************************
#  Creates a hosts file for fidogate from a nodelist
#
#  Author:	 	Thomas Huber (Thomas.Huber@lemas_oschwand.fidonet.org)
#  Version:		0.1
#  Date:		17.09.95
#  Last modified:	1.10.95
#
#  Report bugs etc to:	Thomas.Huber@2:301/101.19 or huber@iamexwi.unibe.ch
# ****************************************************************************

sub usage {
print "ndl2hosts [-h] [-z] [-f zones] [-d domain] [-p parameters] [nodelist]\n
ndl2hosts slurps in a nodelist an prints out a hosts file
to use with fidogate.
\t-h:\t\tPrint this usage text
\t-z:\t\tNodelist on stdin is compressed
\t-f zones:\tOnly process nodes in zones contained in string 'zones'
\t-d domain:\tAppend this domain to hostnames
\t-p parameters:\tParameters for hosts file
\t nodelist:\tIf specified, read from this file, else from stdin\n";
exit 1;
}

require "getopts.pl";
&Getopts('hzd:f:p:') ||  &usage;		# Get options

# Defaults if no options given
$params = "-p";			# -p : generate hostnames with point
$domain = "";			# domain appended to hostnames
$filter = "2";			# Only process nodes of zone 2
$COMPRESS = 0;			# nodelist not compressed

# Process given options
if($opt_h) {
  &usage;
}
if($opt_d) {
  $domain = $opt_d;
}
if($opt_f) {
  $filter = $opt_f;
}
if($opt_p) {
  $params = $opt_p;
}
if($opt_z) {
  $COMPRESS = 1;
}

$nodefile = @ARGV[$#ARGV];			# name of input file
if ($nodefile ne "") {                          # if empty, read from stdin
  if ($nodefile=~/.*\.(gz|Z|z|gzip)/) {		# compressed input
     open(in,sprintf("zcat %s|",$nodefile)) || die "Error opening nodelist\n";
     }
  else {                                        # uncompressed input
     open(in,$nodefile) || die "Error opening nodelist";
     }
  $in = in;
  }
else {                                          # read from stdin
  if ($COMPRESS) {                              # STDIN compressed (-z option)
     open(in,"cat - | gzip -d |");              # pipe stdin through gzip -d
     $in = in;
     }
  else {                                        # stdin uncompressed
     $in = "STDIN";
     }
}


# Perform various substitutions on the hostnames in order to get acceptable
# rfc adresses. Eliminates the generation of ugly adresses from braindead
# nodenames as "(yber**foo(((|3ar>>>]-[otel_23.00-0600++#124-1&2!!"  (-;
sub do_subs {
   local($name) = @_;	
   study($name);
   $name=~tr/[A-Z]/[a-z]/;			# convert all to lowercase

   # Replace foo_bar(IsDn-Blubb) by foo_bar_isdn
   $name=~s/(\(|\[).*(I|i)(S|s)(D|d)(N|n).*(\)|])/isdn/go;    


   $name=~s/\\\\/h/go;				# \\ill-foo -> hill-foo
   $name=~s/\\\\\//w/go;			# \\/orld-bar->world-bar
   $name=~s/\][\\\/=-]\[/h/go;			# ]-[ello -> hello
   $name=~s/\(y/cy/go;				# (yber -> cyber

   # Kill unwanted characters
   $name=~s/([\{\}\[\]~\?\(\)<>=%$\@#\!\/\'\*;:,"`\+]{1,}|\(.*\)|\[.*\])//go;
   $name=~s/&/and/go;				# Replace & by 'and'

   #  Replace   _-_  . _- -_  by -
   $name=~s/(_*-{1,}_*|\.)/-/go;	

   # Replace ------- by -, ____ by _
   $name=~s/-{1,}/-/go;
   $name=~s/_{1,}/_/go;
   $name=~s/={1,}/=/go;

   # Kill times in hostname (bar_box-2300-0600)
   $name=~s/[0-2][0-9][0-5][0-9]-[0-2][0-9][0-5][0-9]//go;
   $name=~s/([0-1])?[0-9][ap]m-([0-1])?[0-9][ap]m//go;
   $name=~s/[0-2][0-9](_)?([0-5][0-9])?(-|_)[0-2][0-9](_)?([0-5][0-9])?//go;

   # remove leading _ - = \ /
   $name=~s/^[-_=\[\]\\\/]{1,}//go;

   # How do you kill trailing _ - etc ? The only way I get it to work
   # is this one:
   $name=reverse($name);
   $name=~s/^[-_=\[\]\\\/]{1,}//go;
   $name=reverse($name);

   return $name;
}



$host=""; 					# init
printf stderr ("Doing substitutions...");
while (<$in>) {
  if (index($_,";") == -1 ) { 			# skip comments
   split(",",$_);				# split at commas
   if ($_[0] eq "Zone") { 			# all following hosts
	$zone=$_[1]; 				# are from this zone
	$host="";
	}
   if ($_[0] eq "Host") { 			# all following nodes are
	$host=$_[1]; 				# from this host
	}

   if (index($filter,$zone) != -1) {
     if (($_[0] ne "Hub")&&($_[0] ne "Region")&&
      ($_[0] ne "Host")&&($_[1] ne "")&&($host ne "")&&
      ($zone ne "")) {
	  $node=$_[1];
	  $name=$_[2];
	  $name=&do_subs($name);
	  # unlucky nodenames are deleted by do_subs, i.e. "(foo-box)"
	  if ($name ne "") {
	    push(@hostsfile,sprintf("%s:%s/%s\t%s\t\n",$zone,$host,$node,
	         $name));
	    }
	  }
     }
  }
}
printf stderr ("done.\n");



# The raw hosts file is now in the array @hostsfile for further processing
# The purpose of the following code is to guarantee that nodenames are unique
# If multiple nodes having the same name are found, append a number to make
# them unique, i.e. foo-1, foo-2, foo-3 etc.
printf stderr ("Sorting...");

# Sort by GNU sort, as perl's sort function is so unefficient that
# it eats up all my memory...
open(temp,">temp_file");
print temp @hostsfile;
open(temp2,"sort -b +1 temp_file |");
$temp2 = temp2;
while (<$temp2>) {
   push(@sorted,$_);
}
system("rm -f temp_file");

# @sorted = sort({(split("\t",$a))[1] cmp (split("\t",$b))[1]} @hostsfile);
printf stderr ("done.\nRemoving duplicates...");

$tmpspace="\t\t\t\t\t\t\t\t";
$counter = "";
$isdupe = 0;
$stroke = "";					# stroke ('-') separates hostname
foreach $tmp(@sorted) {				# and number
	@current=split("\t",$tmp);		# split into adr and hostname
	if (@current[1] eq @previous[1]) {	# same hostnames!
	   if ($counter eq "") { $counter = 0; }# first dupe -> start counting
	   $isdupe = 1;				# found dupe
	   $counter++;				# one more dupe
	}
	else {					# hostname not the same as
	   if ($isdupe) {			# previous hostname
		$isdupe = 0;			# the current host isn't a dupe,
		$counter++;			# but the previous is one
	   }
	}
	if (@previous[0] ne "") {	   	# first loop run ?
	  if ($counter ne "") { 		# counter set -> dupes found
		$stroke = "-"; 			# in hostname, number is
		}				# separated from hostame by -
	  else { 
		$stroke = "";			# no counter, no stroke
		}
	  # calculate number of Tabs for a good looking hosts file
	  $space=substr($tmpspace,0,
	    (8-(length(@previous[1])+length($counter)+length($stroke)
	         +length($domain)+1)/8));
	  # push a line on the output-array
	  push(@out,sprintf("%s \t%s%s%s\n",
	       @previous[0],@previous[1].$stroke.$counter.$domain,$space,$params));
	  if (!$isdupe) { $counter = ""; }	# if prev was a dupe but current
						# isn't delete counter
	}
	@previous = @current;			# proceed to next loop run
}
# the last one
if (@previous) {
 if ($isdupe) {$counter++;}
 $space=substr($tmpspace,0,
	      (8-(length(@previous[1])+length($counter)+length($stroke)
	       +length($domain)+1)/8));
 push(@out,sprintf("%s \t%s%s%s\n",
     @previous[0],@previous[1].$stroke.$counter.$domain,$space,$params));
}
printf stderr ("done.\n");
print @out;					# print the whole shit'out (-:
