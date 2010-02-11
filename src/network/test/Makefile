SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
# ROOT= $(shell cd ../..; pwd)
ROOT= ../..
DEPINC = -I. -I$(ROOT)/common
INCLUDE = $(DEPINC)
DEFS = 

all: medtester

include $(ROOT)/Make.conf

install: 

test: medtester
	@./run.sh 0; sleep 1;\
	./run.sh 1; sleep 1;\
	./run.sh 2; sleep 1;\
	./run.sh 3; sleep 1; \
	echo "########## DONE ###########"

medtester: $(OBJS)
	$(CXX) -o $(@) $(OBJS) \
	$(ROOT)/common/libaacommon.a

clean:
	$(RM) $(OBJS) medtester med*.log test*.log gdb.in

distclean: clean

# DO NOT DELETE

tester.o: ../../common/timing_message.h
