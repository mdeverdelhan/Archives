#!/usr/bin/perl

if ( @ARGV != 2 ) {
	print "Usage:  \n";
	print " create_db.pl <type> <inputfile>\n\n";
	print "type is p for passwords, l for username/login\n\n";
	exit;
} else {
	$type = shift;
	$file = shift;
}

if ( $type eq "p" ){ $typeid = "PASS"; }
if ( $type eq "l" ){ $typeid = "LOGI"; }


open (DAT, ">output.tpdb");

print DAT "file\n filename = \"prutus_$typeid\";\n";
print DAT " attrib = backup;\n";
print DAT " typeid = '$typeid';\n";
print DAT " creatorid = 'PRUT';\n\n";
print DAT "begin\n\n";

open ($input, $file);
while (<$input>) {
	print DAT " record\n begin\n";
	chomp($_);
	print DAT "  \"$_\"\n";
	print DAT " end;\n\n";


}

print DAT "end;\n";
close (DAT);
close ($input);
print "Use pdbc to create the .pdb file\n\n";
