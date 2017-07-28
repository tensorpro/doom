#!/bin/bash	
BASEDIR=$HOME/anaconda2

echo $BASEDIR

./cmake_clean.sh
rm -rf ./bin

cmake . -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON=ON \
-DPYTHON_INCLUDE_DIR=$BASEDIR/include/python2.7 \
-DPYTHON_LIBRARY=$BASEDIR/lib/libpython2.7.so \
-DPYTHON_EXECUTABLE=$BASEDIR/bin/python2.7 \
-DNUMPY_INCLUDES=$BASEDIR/lib/python2.7/site-packages/numpy/core/include

make -j

rm -rf $BASEDIR/lib/python2.7/site-packages/vizdoom
mkdir -p $BASEDIR/lib/python2.7/site-packages/vizdoom
cp -r bin/python2/pip_package/ $BASEDIR/lib/python2.7/site-pcakages/vizdoom
