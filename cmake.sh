#!/bin/bash	

BASEDIR=$CONDA_PREFIX

# Detect operating system
if [[ "$(uname)" == "Darwin" ]]; then
        echo "Using OSX Settings for compilation"
        if [[ $BASEDIR == "" ]]; then
                BASEDIR=$HOME/.anaconda
        fi
        echo $BASEDIR

        F_LIB="dylib"
else
        echo "Using Linux Settings for compilation"
        if [[ $BASEDIR == "" ]]; then
                BASEDIR=$HOME/anaconda2
        fi

        F_LIB="so"
fi

echo $BASEDIR/lib/libpython2.7.$F_LIB

./cmake_clean.sh
rm -rf ./bin

cmake . -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON=ON \
-DPYTHON_INCLUDE_DIR=$BASEDIR/include/python2.7 \
-DPYTHON_LIBRARY=$BASEDIR/lib/libpython2.7.$F_LIB \
-DPYTHON_EXECUTABLE=$BASEDIR/bin/python2.7 \
-DNUMPY_INCLUDES=$BASEDIR/lib/python2.7/site-packages/numpy/core/include

make -j

rm -rf $BASEDIR/lib/python2.7/site-packages/vizdoom
mkdir -p $BASEDIR/lib/python2.7/site-packages/vizdoom
cp -r bin/python2/pip_package/* $BASEDIR/lib/python2.7/site-packages/vizdoom
