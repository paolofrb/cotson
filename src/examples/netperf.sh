#!/bin/bash

NODE=$1
TOTAL_NODES=$2

################# NODE 1 ##################
if [ $NODE -eq 1 ] ; then
   echo "Node #1"
   ping -i 0.2 -c 2 n2
   xget /tmp/netperf /tmp/netperf
   chmod +x /tmp/netperf
   sleep 1  ## wait for netserver to start
   echo "starting netperf"
   /tmp/netperf -v 2 -l 1 -H n2 | tee /tmp/np.log 2>&1
else
################# NODE 2 ##################
   echo "Node #2"
   ping -i 0.2 -c 2 n1
   xget /tmp/netserver /tmp/netserver
   echo "starting netserver"
   chmod +x /tmp/netserver
   /tmp/netserver -v 2
   while true; do
      echo "still working at `date`"
      sleep 1 
   done
fi

