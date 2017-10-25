#!/bin/bash	
BASEDIR=$HOME/.conda
echo $1
rm -rf $BASEDIR/lib/python2.7/site-packages/vizdoom
cp -r bin/python2/pip_package $BASEDIR/lib/python2.7/site-packages/vizdoom
