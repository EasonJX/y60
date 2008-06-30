#!/bin/sh

function onExit {
         echo killing all jobs
         kill `jobs -p`
}

# necessary to remove all jobs of this shell if we get a SIGTERM or
# similar
trap "onExit" EXIT


Y60=`pwd`/..
export PATH=$Y60:$PATH
export LD_LIBRARY_PATH=$Y60:$LD_LIBRARY_PATH

echo $LD_LIBRARY_PATH

LOOP="1"
while [ $LOOP == "1" ]; do
    y60 client.js
    sleep 1
    echo "restarting"
done;
