#!perl -w
use Getopt::Long qw(:config bundling);
use File::Basename;
use File::Find;
use strict;

sub usage {
  my $file = basename($0);
  "usage: $file [options]

This script substitutes hardcoded values for the \$(PlatformName) and
\$(ConfigurationName) variables in the OpenSceneGraph, Producer, and
OpenThreads .dsp files.  Those variables are useful in VC 7.x and VC 8,
but are incompatible with VC 6.0.

To run, open a shell and execute
        perl $file

Options:
  -h, --help               print help information
";
}

my ($print_help);
my $opt_success = GetOptions('h|help'           => \$print_help);

if (!$opt_success || $print_help) { print usage; exit !$print_help; }


# update the entire text block of settings for the given named project
# and configuration
sub update_config_block {
  my ($project_name, $project_config, $is_static, $text) = @_;
  local($_) = $text;

  s/\$\(PlatformName\)/win32/g;
  s/\$\(ConfigurationName\)/$project_config/g;

  $_;
}

sub backup_file {
  my ($file, $backup_file) = @_;
  local($/) = undef;            # slurp whole file

  my $contents;
  open(ORIG, "$file") || die "$!";
  $contents = <ORIG>;
  close(ORIG);

  open(BACKUP, "> $backup_file") || die "$!";
  print BACKUP $contents;
  close(BACKUP);
}


# Update all the configuration text blocks, one-by-one, in the dsp file.
# Backs up existing dsps using a .bak extension.
sub update_dsp {
  my ($project_file) = @_;

  print STDERR $project_file, "\n";
  
  my $backup = $project_file.".bak";
  backup_file($project_file, $backup);

  open(INPUT, "$backup") || die "$!";
  open(OUTPUT, "> $project_file") || die "$!";
  while (defined($_ = <INPUT>)) {
    print OUTPUT $_;
    if (/^!(?:ELSE)?IF  "\$\(CFG\)" == "(.+?) - Win32 (.+)"\s*$/) {
      # start of config block
      
      my ($project, $config) = ($1, $2);
      my $is_static = ($config =~ s/\s*Static\s*$//);

      my $config_text = "";
      <INPUT>;                  # read blank line
      while (defined($_ = <INPUT>) && !/^\s*$/) {
        s/\r?\n/\n/;
        $config_text .= $_;
      }

      $config_text = update_config_block($project, $config, $is_static, $config_text);

      print OUTPUT map "$_\r\n", ("", split(/\n/, $config_text), "");
    }
  }
  close(OUTPUT);
  close(INPUT);
}

sub callback {
  update_dsp($_) if /\.dsp$/i;
}

my (@dirs) = qw(.
                ../../Producer/VC++6.0/Producer
                ../../OpenThreads/win32_src);

# change to directory containing script to ensure paths work properly
chdir dirname($0);

find(\&callback, @dirs);

exit;
