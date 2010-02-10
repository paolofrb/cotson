#!/bin/bash
CPORT=50000
DPORT=50001
MPORT=50002
MCIP=239.200.1.1
mode=$1
echo "Starting mediator"
../mediator CPORT=$CPORT DPORT=$DPORT MPORT=$MPORT med.in | tee med$mode.log 2>&1 & 
sleep 2
echo "Running test"
./medtester localhost $CPORT $MCIP $MPORT 10 10 $mode
