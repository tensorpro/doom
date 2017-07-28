#!/bin/bash	
BASEDIR=$HOME/anaconda2

echo $BASEDIR

make -j

rm -rf $BASEDIR/lib/python2.7/site-packages/vizdoom
mkdir -p $BASEDIR/lib/python2.7/site-packages/vizdoom
cp -r bin/python2/pip_package/* $BASEDIR/lib/python2.7/site-packages/vizdoom/
