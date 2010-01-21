.PHONY: build clean distclean

all: build

.PHONY: run run_%
EXAMPLES= \
	functional \
	mem_tracer trace_stats  \
	one_cpu_simple smarts dynamic multiple_cpu_interval \
	twonodes \
	tracer

run: $(addprefix run_, $(EXAMPLES))

run_functional:
	@echo "Simple example how to use COTSon for interactive simulation."; \
	echo "Try running some simnow commands and then 'quit' to exit simulation"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	../../bin/cotson functional.in

run_mem_tracer:
	@echo "The mem_tracer outputs a memory trace to /tmp/mem_tracer.txt.gz"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	../../bin/cotson mem_tracer.in; \
	zless /tmp/mem_tracer.txt.gz

run_trace_stats:
	@echo "The trace_stats example outputs a disassembled trace to "; \
    echo "/tmp/trace_stats.0.txt.gz (one per CPU)"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	../../bin/cotson trace_stats.in; \
	zless /tmp/trace_stats.0.txt.gz

run_twonodes:
	@echo "Running a cluster of 2 nodes pinging each other"; \
	echo "Simulation runs in background -- be patient (a few minutes)"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	( ../../bin/cotson twonodes.in & ) ; \
	sleep 10; \
	echo "We now fire a viewer of the two consoles"; \
	./twonodes-ctrl view

run_sqlite:
	@echo "Running 3 times an experiment (different samplers) writing to sqlite DB"; \
	echo "At the end, the IPC is plotted"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	/bin/rm -f /tmp/test.db; \
	../../bin/cotson EXP=1 sqlite.in; \
	../tools/ipcplot file:/tmp/test.db 1 -g 0.01 -r 0; \
	../../bin/cotson EXP=2 sqlite.in; \
	../tools/ipcplot file:/tmp/test.db 2 -g 0.01 -r 0; \
	../../bin/cotson EXP=3 sqlite.in; \
	../tools/ipcplot file:/tmp/test.db 3 -g 0.01 -r 0; \

run_%:
	@echo "Running the $* example"; \
	echo "When done, look at node.*.$*.log*"; \
	echo "Type <return> to start or <ctrl-c> to abort"; read a; \
	../../bin/cotson $*.in

run_tracer:
	$(MAKE) -C tracer

build: ../abaeterno/abaeterno.so
	ln -fs ../abaeterno/abaeterno.so .

install: 

clean:
	/bin/rm -f *log* abaeterno.so exp-*.gp exp-*.dat
	make -C tracer clean

distclean: clean


