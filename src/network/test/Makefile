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
	@./run.sh

medtester: $(OBJS)
	$(CXX) -o $(@) $(OBJS) \
	$(ROOT)/common/libaacommon.a

clean:
	$(RM) $(OBJS) medtester med.log med-sim.log

distclean: clean

# DO NOT DELETE

tester.o: ../../common/timing_message.h
