#!/bin/bash

# ECM: script for calling python tests that check user-side pynn api;
#      no handling of (server-side) errors yet...

PYNNTESTDIR=`dirname $0`
if [ ! -d $PYNNTESTDIR ]; then
    exit 1
fi
export PYNNTESTDIR

# ah, ugly shit
if [ -e bindings/libeuter.so ]; then
	cp bindings/libeuter.so $PYNNTESTDIR/
fi


# enumerate test scripts here :)

"$PYNNTESTDIR"/test_numpy_param.sh
"$PYNNTESTDIR"/test_large_network_01.sh
#...
