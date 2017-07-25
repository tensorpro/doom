#!/bin/bash	
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON=ON -DPYTHON_INCLUDE_DIR=$HOME/.miniconda2/include/python2.7 -DPYTHON_LIBRARY=$HOME/.miniconda2/lib/libpython2.7.so  -DPYTHON_EXECUTABLE=$HOME/.miniconda2/bin/python2.7 -DNUMPY_INCLUDES=$HOME/.miniconda2/lib/python2.7/site-packages/numpy/core/include
