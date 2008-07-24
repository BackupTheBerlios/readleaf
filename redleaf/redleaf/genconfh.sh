#!/bin/sh

OFILE=config.h

DSTDIRo=`cat config | grep DESTDIR | awk {'print $3'}`
VMJ=`cat config | grep VER_MAJOR | awk {'print $3'}`
VMM=`cat config | grep VER_MINOR | awk {'print $3'}`
VSUF=`cat config | grep VER_SUFFIX | awk {'print $3'}`
VTR=`cat config | grep VER_TREE | awk {'print $3'}`

DSTDIR=`echo $DSTDIRo | sed 's|\/|\\\/|g'`

cat config.h.in | sed s/%DSTDIR/$DSTDIR/g > $OFILE
sed -i s/%VMJ/$VMJ/g $OFILE 
sed -i s/%VMM/$VMM/g $OFILE 
sed -i s/%VSUF/$VSUF/g $OFILE 
sed -i s/%VTR/$VTR/g $OFILE 


