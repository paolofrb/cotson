.PHONY: build clean distclean

all: build

.PHONY: run run_%
EXAMPLES= \
	functional \
	mem_tracer mem_tracer2 trace_stats  \
	one_cpu_simple smarts dynamic multiple_cpu_interval \
	twonodes twohet \
	tracer

run: $(addprefix run_, $(EXAMPLES))
cleanup: $(addprefix cleanup_, $(EXAMPLES))

run_functional:
	@echo "Simple example how to use COTSon for interactive simulation."; \
	echo "Try running some simnow commands and then 'quit' to exit simulation"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	../../bin/cotson functional.in

cleanup_functional:

run_slirp:
	@echo "Simple example how to use COTSon with the building slirp (NAT) interface."; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	../../bin/cotson slirp.in

cleanup_slirp:


run_mem_tracer:
	@echo "The mem_tracer outputs a memory trace to /tmp/mem_tracer.txt.gz"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	../../bin/cotson mem_tracer.in; \
	zless /tmp/mem_tracer.txt.gz

cleanup_mem_tracer:

run_mem_tracer2:
	@echo "The mem_tracer2 outputs a memory trace to /tmp/mem_tracer2.txt.gz"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	../../bin/cotson mem_tracer2.in; \
	zless /tmp/mem_tracer2.txt.gz

cleanup_mem_tracer2:

run_trace_stats:
	@echo "The trace_stats example outputs a disassembled trace to "; \
	echo "/tmp/trace_stats.0.txt.gz (one per CPU)"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	../../bin/cotson trace_stats.in; \
	zless /tmp/trace_stats.0.txt.gz

cleanup_trace_stats:

run_twonodes:
	@echo "Running a cluster of 2 nodes pinging each other"; \
	echo "Simulation runs in background -- be patient (a few minutes)"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	isoinfo -x '/COTSON_G.TGZ;1' -i ../../data/cotson-guest-tools.iso|tar xzf - usr/bin/cotson_tracer -O >/tmp/cotson_tracer; \
	/bin/rm -f run_twonodes.log /tmp/twonodes.db; \
	( (sleep 10; if [ -s ./twonodes-ctrl ]; then echo "Firing console view"; ./twonodes-ctrl view; fi) &); \
	../../bin/cotson twonodes.in > run_twonodes.log 2>&1 ; \
	if [ -s /tmp/net-trace.gz ]; then echo "Dumping the network trace"; \
	../tools/traceplayer /tmp/net-trace.gz 0 1; fi

cleanup_twonodes:
	@echo "Cleanup twonodes example..."; \
	/bin/rm -rf twonodes-* twonodes-me* twonodes-no* run_twonodes.log /tmp/X* /tmp/.X* err.med.*.log err.node.*.log twonodes.*.log /tmp/twonodes.db /tmp/net-trace.gz /tmp/vnc* ; \
	ps axuc |awk '/vnc|screen|simnow/{print $2}'| xargs --no-run-if-empty sudo kill -9; \
	sudo ifconfig virbr0 down; \
        echo "...done."

run_twonodes_novnc:
	@echo "Running a cluster of 2 nodes pinging each other (NO VNC)"; \
	echo "Simulation runs in background -- be patient (a few minutes)"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	isoinfo -x '/COTSON_G.TGZ;1' -i ../../data/cotson-guest-tools.iso|tar xzf - usr/bin/cotson_tracer -O >/tmp/cotson_tracer; \
	/bin/rm -f run_twonodes_novnc.log /tmp/twonodes_novnc.db; \
	( (sleep 10; if [ -s ./twonodes_novnc-ctrl ]; then echo "Firing console view"; ./twonodes_novnc-ctrl;  fi) &); \
	../../bin/cotson twonodes_novnc.in > run_twonodes_novnc.log 2>&1 ; \
	if [ -s /tmp/net-trace.gz ]; then echo "Dumping the network trace"; \
	../tools/traceplayer /tmp/net-trace.gz 0 1; fi

cleanup_twonodes_novnc:
	@echo "Cleanup twonodes_novnc example..."; \
	/bin/rm -rf twonodes_novnc-* twonodes_novnc-me* twonodes_novnc-no* run_twonodes_novnc.log /tmp/X* /tmp/.X* err.med.*.log err.node.*.log twonodes_novnc.*.log /tmp/twonodes_novnc.db /tmp/net-trace.gz /tmp/vnc* ; \
	ps axuc |awk '/vnc|screen|simnow/{print $2}'| xargs --no-run-if-empty sudo kill -9; \
	sudo ifconfig virbr0 down; \
        echo "...done."

run_twonodes_noscreen:
	@echo "Running a cluster of 2 nodes pinging each other (NO SCREEN)"; \
	echo "Simulation runs in background -- be patient (a few minutes)"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	isoinfo -x '/COTSON_G.TGZ;1' -i ../../data/cotson-guest-tools.iso|tar xzf - usr/bin/cotson_tracer -O >/tmp/cotson_tracer; \
	/bin/rm -f run_twonodes_noscreen.log /tmp/twonodes_noscreen.db; \
	( (sleep 10; if [ -s ./twonodes_noscreen-ctrl ]; then echo "Firing console view"; ./twonodes_noscreen-ctrl;  fi) &); \
	../../bin/cotson twonodes_noscreen.in > run_twonodes_noscreen.log 2>&1 ; \
	if [ -s /tmp/net-trace.gz ]; then echo "Dumping the network trace"; \
	../tools/traceplayer /tmp/net-trace.gz 0 1; fi

cleanup_twonodes_noscreen:
	@echo "Cleanup twonodes_noscreen example..."; \
	/bin/rm -rf twonodes_noscreen-* twonodes_noscreen-me* twonodes_noscreen-no* run_twonodes_noscreen.log /tmp/X* /tmp/.X* err.med.*.log err.node.*.log twonodes_noscreen.*.log /tmp/twonodes_noscreen.db /tmp/net-trace.gz /tmp/vnc* ; \
	ps axuc |awk '/vnc|screen|simnow/{print $2}'| xargs --no-run-if-empty sudo kill -9; \
	sudo ifconfig virbr0 down; \
        echo "...done."

run_twohet:
	@echo "Running a cluster of 2 heterogeneous nodes pinging each other"; \
	echo "Simulation runs in background -- be patient (a few minutes)"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	isoinfo -x '/COTSON_G.TGZ;1' -i ../../data/cotson-guest-tools.iso|tar xzf - usr/bin/cotson_tracer -O >/tmp/cotson_tracer; \
	/bin/rm -f run_twohet.log /tmp/twohet.db; \
	( (sleep 10; echo "Firing console view"; ./twohet-ctrl view) &); \
	../../bin/cotson twohet.in > run_twohet.log 2>&1 

cleanup_twohet:
	@echo "Cleanup twohet example..."; \
	/bin/rm -rf twohet-* twohet-me* twohet-no* run_twohet.log /tmp/X* /tmp/.X* err.med.*.log err.node.*.log twohet.*.log /tmp/twohet.db /tmp/net-trace.gz /tmp/vnc* ; \
	ps axuc |awk '/vnc|screen|simnow/{print $2}'| xargs --no-run-if-empty sudo kill -9; \
	sudo ifconfig virbr0 down; \
        echo "...done."

run_twonodes_interact:
	@echo "Running a cluster of 2 nodes (INTERACTIVE)"; \
	echo "Simulation runs in background -- be patient (a few minutes)"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	isoinfo -x '/COTSON_G.TGZ;1' -i ../../data/cotson-guest-tools.iso|tar xzf - usr/bin/cotson_tracer -O >/tmp/cotson_tracer; \
	/bin/rm -f run_twonodes_interact.log /tmp/twonodes_interact.db; \
	../../bin/cotson twonodes_interact.in > run_twonodes_interact.log 2>&1 ;

cleanup_twonodes_interact:
	@echo "Cleanup twonodes_interact example..."; \
	/bin/rm -rf twonodes_interact-* twonodes_interact-me* twonodes_interact-no* run_twonodes_interact.log /tmp/X* /tmp/.X* err.med.*.log err.node.*.log twonodes_interact.*.log /tmp/twonodes_interact.db /tmp/net-trace.gz /tmp/vnc* ; \
	ps axuc |awk '/vnc|screen|simnow/{print $2}'| xargs --no-run-if-empty sudo kill -9; \
	sudo ifconfig virbr0 down; \
        echo "...done."


run_netperf:
	@echo "Running the netperf benchmark (two nodes)"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	tar xfj np.tbz -C /tmp; \
	rm -rf run_netperf.log /tmp/netperf.db; \
	../../bin/cotson netperf.in > run_netperf.log 2>&1; \
	../tools/ipcplot file:/tmp/netperf.db 1 -g 0.1 -mm ipc -r 0; \
	../tools/ipcplot file:/tmp/netperf.db 1 -g 0.1 -mm packets -r 0

cleanup_netperf:
	@echo "Cleanup netperf example..."; \
	/bin/rm -rf np-* np-me* np-no* run_np.log /tmp/X* /tmp/.X* err.med.*.log err.node.*.log np.*.log /tmp/np.db /tmp/net-trace.gz /tmp/vnc* ; \
	ps axuc |awk '/vnc|screen|simnow/{print $2}'| xargs --no-run-if-empty sudo kill -9; \
	sudo ifconfig virbr0 down; \
        echo "...done."

run_sqlite:
	echo "Running 3 times an experiment (different samplers) writing to sqlite DB"; \
	echo "At the end, the IPC is plotted"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	/bin/rm -f /tmp/test.db; \
	(( ../../bin/cotson EXP=1 sqlite.in; \
	  ../tools/ipcplot file:/tmp/test.db 1 -g 0.01 -r 0 ) & );  \
	(( ../../bin/cotson EXP=2 sqlite.in; \
	  ../tools/ipcplot file:/tmp/test.db 2 -g 0.01 -r 0 ) & ); \
	(( ../../bin/cotson EXP=3 sqlite.in; \
	  ../tools/ipcplot file:/tmp/test.db 3 -g 0.01 -r 0 ) & ); \
	  wait

cleanup_sqlite:

run_%:
	@echo "Running the $* example"; \
	echo "When done, look at node.*.$*.log*"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	../../bin/cotson $*.in

cleanup_%:
	@echo "Cleanup $* example..."; \
	echo "...done"

run_tracer:
	$(MAKE) -C tracer

cleanup_tracer:
	@echo "Cleanup..."; \
	$(MAKE) -C tracer clean; \
	echo "...done"

build: ../abaeterno/abaeterno.so
	ln -fs ../abaeterno/abaeterno.so .

install: 

dep:

clean:
	/bin/rm -rf *log* abaeterno.so exp-*.gp exp-*.dat twonodes-*
	make -C tracer clean

distclean: clean


