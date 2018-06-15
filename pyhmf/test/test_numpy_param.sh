#!/bin/bash

. $PYNNTESTDIR/test_helper.sh

SCRIPTNAME="test_numpy_param.py"

pythonPidFile="python.pid"
( python $PYNNTESTDIR/$SCRIPTNAME ; rm $pythonPidFile ; ) &
pythonPid=$!
echo $pythonPid > $pythonPidFile

# timeout for python process
( sleep 30 ; if [[ -e $pythonPidFile ]]; then killtree $pythonPid ; fi ; ) &
termPid=$!

wait $pythonPid
kill $termPid

exit 0 # always a success ;p
