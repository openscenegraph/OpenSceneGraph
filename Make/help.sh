#!/bin/sh

OS=$1
TOPDIR=$2
INST_LIBS=$3
INST_PLUGINS=$4
INST_INCLUDE=$5
INST_EXAMPLES=$6
INST_EXAMPLE_SRC=$7
INST_DOC=$8
INST_DATA=$9


cat <<- EOF

The following targets can be used with any subsystem as well as the top
level build (build in the OSG Root directory).  Note that debug versions 
and optimized version of targets reside in parallel.

  make                - Same as 'make opt'
  make opt            - Makes optimized versions of all targets
  make debug          - Makes debug versions of all targets.
  make clean          - Removes all object files (both optimized and debug 
                        versions) and Makedepend files.
  make cleanopt       - Removes optimized version of object files and 
                        Makedepend files.
  make cleandbg       - Removes debug version of object files and Makedepend
                        files.
  make cleandepend    - Removes Makedepend file(s) (both optimized and debug 
                        versions)
  make cleandependopt - Removes optimized version Makedepend file(s) 
  make cleandependdbg - Removes debug version Makedepend file(s)
  make cleantarget    - Removes only targets but leaves object files (both
                        optimized and debug versions)
  make cleantargetopt - Removes only optimized targets but leaves optimized
                        objects files.
  make cleantargetdbg - Removes only debug targets but leaves debug object
                        files
  make clobber        - Removes object files and targets (both optimized and
                        debug versions)
  make clobberopt     - Removes optimized object files and targets
  make clobberdbg     - Removes debug object files and targets
  make beautify       - Pretty print C++ files
  make docs           - Builds documentation database for current target
  make depend         - Force a rebuild of the dependency file.  Note that
                        dependency files are generated automatically during
                        builds.


Solaris, IRIX and Linux (some compilers) can build 64 bit targets.  These 
require the ARCH=64 argument.  For example:

  make ARCH=64         - Same as 'make ARCH=64 opt'
  make ARCH=64 opt     - Builds 64 bit optimized targets
  make ARCH=64 debug   - Builds 64 bit debug versions of targets
  make ARCH=64 clean   - Removes all 64 bit object files (both optimized and
                         debug versions).

  etc.

The following targets may only be issued from the top-level OSG build:

  make install      - Install both execution environment and development
                      targets 
  make instbin      - Install execution environment targets only.  These
                      consist of libraries, plugins and example programs.
                      Libraries are installed in
                        o $INST_LIBS,
                      plugins are installed in 
                        o $INST_PLUGINS,
                      and examples are installed in 
                        o $INST_EXAMPLES
                      on $OS

  make instdev      - Install development targets only.  These consist of
                      header files, source code to the example programs, and
                      documentation.
                      Header files are install in 
                        o $INST_INCLUDE,
                      example source code is installed in
                        o $INST_EXAMPLE_SRC,
                      and documentation is installed in
                        o $INST_DOC
                      on $OS

  make instlinks    - Installs symbolic links at install locations for both
                      execution environment and development targets rather
                      than copyied files.  Installing links is ideal for a
                      development environment for avoiding confusion about
                      which binaries are being run or linked to.

  make instlinksbin  - Installs symbolic links at install locations for
                       execution environment targets only.
  make instlinksdev  - Installs symbolic links at install locations for
                       development targets only
  make instclean     - Removes installed targets (files or links) from 
                       installation locations for both execution environment
                       and development targets
  make instcleanbin  - Removes installed targets from installation locations
                       for execution environment targets only
  make instcleandev  - Removes installed targets from installation locations
                       for defelopment targets only

Note that the following variables can be used to directly override the default 
installation locations for $OS.

  make  INST_LIBS=<libs_location> \\
        INST_PLUGINS=<plugins_location>\\
        INST_INCLUDE=<header_file_location>\\
        INST_EXAMPLES=<examples_location>\\
        INST_EXAMPLE_SRC=<example_src_location>\\
        INST_DOC=<doc_location>\\
        install

                     - Installs libraries in <libs_location>, plugins in 
                       <plugins_location>, header files in 
                       <header_file_location>, examples in <examples_location>,
                       example source code in <example_src_location> and 
                       documentation in <doc_location>

Note also that INST_LIBS, INST_PLUGINS, INST_INCLUDE, and INST_SHARE share 
a common prefix by default: INST_LOCATION. Further INST_EXAMPLES, INST_EXAMPLE_SRC, 
INST_DOC, and INST_DATA share a common prefix by default : INST_SHARE, which
is located under INST_LOCATION by default.  This provides a short cut for the 
above 'make' usage.  For example,

  make INST_LOCATION=/usr/local/OpenSceneGraph \\
       INST_SHARE=/usr/share/OpenSceneGraph \\
       install


These values can be tested by reissuing 'make help' with these arguments.

After doing a 'make install' or 'make instlinks', and if not already added, 
add

    $INST_EXAMPLES

to your PATH environmental variable to run the examples.  If it is not already 
present, add

    $INST_LIBS

and  

    $INST_PLUGINS

to your LD_LIBRARY_PATH environmental variable.  When compiling programs 
using OSG headers add to your -I compile flags:

    $INST_INCLUDE

EOF

exit 0
