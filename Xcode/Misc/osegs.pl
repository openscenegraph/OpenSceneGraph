#!/usr/bin/perl

die "Usage: $0 file\n" unless @ARGV;
open(OTOOL, "otool -l $ARGV[0]|") || die "Can't run otool\n";

$cmd      = 0;
$name     = "";
$segstart = 0;
$segend   = 0;
$ostart   = 0xffffffff;
$oend     = 0;

while(<OTOOL>) {
	chop;
	if(/^Load *command /) {
		$cmd = 1;
		next;
	}
	next unless $cmd != 0;
	if(/^ *cmd  *(.*)/ && $cmd == 1) {
		if($1 eq "LC_SEGMENT") {
			$cmd = 2;
		} else {
			$cmd = 0;
		}
		next;
	}
	if(/^ *segname  *(.*)/ && $cmd == 2) {
		$name = $1;
		next;
	}
	if(/^ *vmaddr  *(.*)/ && $cmd == 2) {
		$segstart = hex $1;
		next;
	}
	if(/^ *vmsize  *(.*)/ && $cmd == 2) {
		$len    = hex $1;
		$segend = $segstart + $len;
		printf("Segment $name, %x - %x\n", $segstart, $segend);
		$ostart = $segstart if $segstart < $ostart;
		$oend = $segend if $segend > $oend;
		$cmd = 0;
	}
}

close OTOOL;
$len = $oend - $ostart;
printf("Image from %x to %x (length %x)\n", $ostart, $oend, $len);

exit 0

