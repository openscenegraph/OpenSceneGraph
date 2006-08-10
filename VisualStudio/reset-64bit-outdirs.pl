#!/usr/bin/perl -w
use Getopt::Long qw(:config bundling);
use File::Find;
use File::Basename;
use strict;

sub usage {
  my $file = basename($0);
  "usage: $file [options]

Resets the 64 bit OutputDirectory settings in the .vcproj files for
OpenSceneGraph, Producer, and OpenThreads to be the same as the Win32
settings.  This is a workaround for Visual Studio's annoying practice of
generating this setting itself when creating the 64 bit configuration,
rather than reusing the Win32 setting.

To run, open a shell and execute
        perl $file

options:
  -h, --help               print help information
";
}

my ($print_help);
my $opt_success = GetOptions('h|help'           => \$print_help);

if (!$opt_success || $print_help) { print usage; exit !$print_help; }

sub update_file {
  my ($filename) = @_;

  open(VCPROJ, "$filename") || die "$!";
  local($/) = undef;
  local($_) = <VCPROJ>;
  close(VCPROJ);

  # update the 64 bit DLL output path
  my ($win32_dll_outputdir) = /^\s+Name="Debug\|Win32"\s+OutputDirectory="(.+?)"/mi;
  s/^(\s+Name="(?:Debug|Release)?\|(?:x64|ia64)"\s+OutputDirectory=)"(.+?)"/$1"$win32_dll_outputdir"/mig;

  # update the 64 bit lib output path, if applicable
  my ($win32_lib_outputdir) = /^\s+Name="Debug Static\|Win32"\s+OutputDirectory="(.+?)"/mi;
  s/^(\s+Name="(?:Debug|Release) Static?\|(?:x64|ia64)"\s+OutputDirectory=)"(.+?)"/$1"$win32_lib_outputdir"/mig
    if $win32_lib_outputdir;

  print $filename, "\n";

  open(VCPROJ_OUT, "> $filename") || die "$!";
  print VCPROJ_OUT $_;
  close(VCPROJ_OUT);
}

sub callback {
  update_file($_) if /\.vcproj$/i;
}

# cd to the directory containing this script, so paths below work properly
chdir dirname($0);

my @directories = qw(../../OpenThreads/win32_src
                     ../../Producer/VC++7/Producer
                     .);
find(\&callback, @directories);

exit;
