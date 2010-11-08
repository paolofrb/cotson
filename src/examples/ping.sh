#!/bin/sh 

NODE=$1
TOTAL_NODES=$2

################# NODE 1 ##################
if [ $NODE -eq 1 ] ; then
   echo "Node #1"
   ping -i 0.2 -c 2 n2 | tee /tmp/ping.log
   cotson_tracer 100 200 300 ## Example cpuid sent across the network 
   ping -i 0.2 -c 3 n2 | tee -a /tmp/ping.log
else
################# NODE 2..N ##################
   echo "Node #2"
   ping -c 1 n1
   while true; do
      echo "still working at `date`"
      sleep 1 
   done
fi
