#!/bin/sh

bye()
{
    echo $1
    exit 1
}

TOPDIR=$1
LNSRC=$2
LNDEST=$3
LINKCMD=$4

## Check command line


[ $# -lt 4 ] && bye "Usage: $0 <Top Directory> <Link Source> <Link Destination> <Link command>"

## Check for existence of link source file
[ -n "$LNSRC" ] || bye "Internal error (LNSRC definition)"

## Check for existence of link destination file
[ -n "$LNDEST" ] || bye "Internal error (LNDEST definition)"

if diff -s $TOPDIR/$LNDEST $LNSRC >/dev/null 2>&1
then
    echo "  =====> $LNSRC and $TOPDIR/$LNDEST are in sync"
else
    echo "  =====> resetting $TOPDIR/$LNDEST to point to $LNSRC"

    ## At this point, we must create a link at the link destination
    ## which points back to our link source.  This requires us to
    ## build a path FROM the destination back to the source
  
    #1) Get the source directory as an absolute path
    SRCDIR=`pwd`/`dirname $LNSRC`
   
    #2) Get the destination directory as an absolute path (TOPDIR is
    ##  a relative path from the current directory).
    cd $TOPDIR
    ROOTDIR=`pwd`
    DESTDIR=`dirname "$ROOTDIR"/"$LNDEST"`

    #3) Build a ../../ chain from the destination directory to the
    ## current directory (ROOTDIR), which will become the prefix to
    ## the path
    T=`dirname $LNDEST`
    while [ "$T" != "." ] 
    do 
        T=`dirname $T`;PFX=../"$PFX"
    done

    #4) strip the absolute path prefix of SRCDIR leading to the current
    ## directory, so we are left with the relative path from the current
    ## directory to the link source directory.  Prefix that with PFX.
    LINK=`echo $LINK_ECHO_OPTION $SRCDIR | sed -e "s,$ROOTDIR/,$PFX,"`/`basename $LNDEST`	

    #5) Create the links by changing to the destination directory,
    ## removing any old versions of the link and creating a new one
    [ -d `dirname $LNDEST` ] || mkdir -p `dirname $LNDEST`
    cd `dirname $LNDEST`
    rm -f `basename $LNDEST`
    $LINKCMD $LINK `basename $LNDEST`
fi

exit 0
