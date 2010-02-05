SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
INCLUDE = -I. -I/usr/include/lua5.1 -I../libluabind
DEPINC = -I.
XOPTS = -Werror

all: libaacommon.a test

include ../Make.conf 

install:

libaacommon.a: $(OBJS)
	$(RM) $@; $(AR) $@ $(OBJS)

clean:
	$(RM) $(OBJS) libaacommon.a; \
	echo "Clean test.unit"; \
	$(RM) $(TESTSRC:.cpp=) $(TESTSRC:.cpp=.log) $(TESTSRC:.cpp=.o) \
	      test.unit/make.deps \

distclean: clean

##################################################################################
.PHONY: test
TESTSRC=$(wildcard test.unit/*.cpp)
TESTS=$(patsubst %.cpp,%,$(notdir $(TESTSRC)))
test: test.unit/make.deps $(addprefix run_, $(TESTS))

test.unit/make.deps: always
	perl test.unit/testunitdep.pl > $@

run_%:
	@echo "######## TEST $*"; \
	$(MAKE) -f test.unit/Makefile TSRC=test.unit/$*.cpp \
	        CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" \
                LIBS="../libluabind/libluabind.a -llua5.1"

## Note: some hosts need -lboost_unit_test_framework
##################################################################################


# DO NOT DELETE

aggregator.o: aggregator.h metric.h error.h liboptions.h kill.h static_set.h
aggregator.o: lua_register.h ./lua.hpp units.h cast.h uint64_t_hash.h
aggregator.o: parameters.h param_factory.h options.h
cast.o: cast.h units.h ./lua.hpp
file_hb.o: heartbeat.h metric.h aggregator.h liboptions.h kill.h static_set.h
file_hb.o: lua_register.h ./lua.hpp units.h cast.h uint64_t_hash.h parameters.h
file_hb.o: param_factory.h options.h
file_hb_last.o: heartbeat.h metric.h aggregator.h liboptions.h kill.h
file_hb_last.o: static_set.h lua_register.h ./lua.hpp units.h cast.h
file_hb_last.o: uint64_t_hash.h parameters.h param_factory.h options.h
heartbeat.o: heartbeat.h metric.h aggregator.h liboptions.h kill.h static_set.h
heartbeat.o: lua_register.h ./lua.hpp units.h cast.h uint64_t_hash.h
heartbeat.o: parameters.h param_factory.h options.h
mac.o: mac.h
memgate.o: memgate.h liboptions.h kill.h static_set.h lua_register.h ./lua.hpp
memgate.o: units.h cast.h uint64_t_hash.h parameters.h param_factory.h
memgate.o: options.h error.h
options.o: options.h cast.h units.h ./lua.hpp parameters.h param_factory.h
options.o: kill.h static_set.h fileutil.h
packet.o: packet.h mac.h port.h ../slirp/libslirp.h
parameters.o: parameters.h cast.h units.h ./lua.hpp param_factory.h kill.h
parameters.o: options.h static_set.h
pgsql_hb.o: error.h liboptions.h kill.h static_set.h lua_register.h ./lua.hpp
pgsql_hb.o: units.h cast.h uint64_t_hash.h parameters.h param_factory.h
pgsql_hb.o: options.h libmetric.h metric.h tagged_metric.h heartbeat.h
pgsql_hb.o: aggregator.h
port.o: port.h
sqlite_hb.o: error.h liboptions.h kill.h static_set.h lua_register.h ./lua.hpp
sqlite_hb.o: units.h cast.h uint64_t_hash.h parameters.h param_factory.h
sqlite_hb.o: options.h libmetric.h metric.h tagged_metric.h heartbeat.h
sqlite_hb.o: aggregator.h fileutil.h
sum_aggregator.o: aggregator.h metric.h liboptions.h kill.h static_set.h
sum_aggregator.o: lua_register.h ./lua.hpp units.h cast.h uint64_t_hash.h
sum_aggregator.o: parameters.h param_factory.h options.h
tagged_metric.o: tagged_metric.h metric.h
timing_message.o: timing_message.h
uint64_t_hash.o: uint64_t_hash.h
units.o: units.h
