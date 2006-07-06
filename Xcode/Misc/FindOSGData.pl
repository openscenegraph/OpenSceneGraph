#!/usr/bin/perl -w

# Author: Eric Wing
# 
# This calls mdfind to find Folders on your system called 
# "OpenSceneGraph-Data*" on your system. If successful, 
# the program will sort entries by most recently changed
# and suggest a copy command for you to cut-and-paste if 
# correct.
# Usage: perl FindOSGData.pl 
# To suppress list of all possible matches use:
# perl FindOSGData.pl --single 
# Special: To find LICENCE_GDAL.rtf use:
# perl FindOSGData.pl [--single] LICENSE_GDAL.rtf

use strict;
use warnings;

my $SHOULD_ONLY_PRINT_SUGGESTION = 0;
my $MDFIND_SEARCH_CRITERIA = "kMDItemDisplayName == 'OpenSceneGraph-Data'w && kMDItemKind=Folder";
my $SUGGESTED_COPY_TO_PATH = "PackageDir/Resources";
my $AM_COPYING_DIR = 1;

# Quick and dirty extract file options
if(scalar(@ARGV))
{
	foreach my $item(@ARGV)
	{
		if($item eq "--single")
		{
			$SHOULD_ONLY_PRINT_SUGGESTION = 1;
		}
		elsif($item eq "LICENSE_GDAL.rtf")
		{
			$MDFIND_SEARCH_CRITERIA = "kMDItemDisplayName == 'LICENSE_GDAL.rtf' && kMDItemKind='Rich Text Format (RTF) document'";
			$SUGGESTED_COPY_TO_PATH = "PackageDir";
			$AM_COPYING_DIR = 0;
		}
		else
		{
			print("Unknown argument: $item\n");
		}
	}

}
		

sub main()
{
	# Call mdfind and return the list of files to an array
	print("Calling mdfind (Spotlight)...\n");

	my @filelist = `/usr/bin/mdfind "$MDFIND_SEARCH_CRITERIA"` or die "Couldn't find anything that matched criteria on your system using Spotlight\n";
	my %folder_lastused_map;
	
	#print(@filelist);
	
	foreach my $file(@filelist)
	{
		my $escaped_string = $file;
		# Need to escape all the spaces in the file name
		# (and kill trailing newline if there)
		$escaped_string =~ s/ /\\ /g;
		chomp($escaped_string);

		# Call mdls on each file to get the last changed date
		my $ret_string = `/usr/bin/mdls -name kMDItemFSContentChangeDate $escaped_string`;
		if( $ret_string =~ m/^.*?\nkMDItem.*?=\s+(.*)/ )
		{
			# extract the date string (and kill trailing newline if there)
			my $date_string = $1;
			chomp($date_string);

			#print("Date string: $date_string\n");
			# copy the date string to the map
			$folder_lastused_map{$escaped_string} = $date_string;
		}
		else
		{
			print "Ooops, no match...mdls format may have changed";
		}
	}

	my @sorted_by_most_recently_changed = sort {$folder_lastused_map{$b} cmp $folder_lastused_map{$a}} keys(%folder_lastused_map);
	
	if(0 == scalar(@sorted_by_most_recently_changed))
	{
		print("No matches for OpenSceneGraph-Data were found.");
		exit;
	}

	if(not $SHOULD_ONLY_PRINT_SUGGESTION)
	{
		print("This is the list of possible matches sorted by most recently changed:\n");
		foreach my $file(@sorted_by_most_recently_changed)
		{
			print("$file\n");
		}
	}

	print("\nIf the following file is correct, you may want to copy and paste this line:\n");

	my $item = $sorted_by_most_recently_changed[0];
	# need to escape string
	$item =~ s/ /\\ /g;

	if($AM_COPYING_DIR == 1)
	{
		print("/Developer/Tools/CpMac -r $item/* $SUGGESTED_COPY_TO_PATH\n");
	}
	else
	{
		print("/Developer/Tools/CpMac -r $item $SUGGESTED_COPY_TO_PATH\n");
	}
}

main()


