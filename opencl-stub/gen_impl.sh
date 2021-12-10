#!/bin/bash -e

wraplib=$1

if [ -z "$wraplib" ]; then
    echo "usage: $0 <wraplib repo path (git-core:brain-user/wraplib)>"
    exit 1
fi

inc=$(readlink -f $(dirname $0)/include)
src=$(readlink -f $(dirname $0)/src)

# chdir to wraplib to avoid generating lextab.py
cd $wraplib

./wraplib.py $inc/MCL/cl.h --cpp-args="-I$inc" \
    --cpp-args=-I$wraplib/pycparser-git/utils/fake_libc_include \
    --cpp-args='-D__attribute__(...)=' \
    --cpp-args='-nostdinc' --cpp-args=-U__SSE__ \
    --cpp-args=-U__SSE2__ --cpp-args=-U__AVX__ --cpp-args=-U__MMX__ \
    --cpp-args='-D__extension__=' \
    > $src/libopencl-wrap.h
