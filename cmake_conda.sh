#!/bin/bash	
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON=ON -DPYTHON_INCLUDE_DIR=$HOME/$1/include/python2.7 -DPYTHON_LIBRARY=$HOME/$1/lib/libpython2.7.so  -DPYTHON_EXECUTABLE=$HOME/$1/bin/python2.7 -DNUMPY_INCLUDES=$HOME/$1/lib/python2.7/site-packages/numpy/core/include
