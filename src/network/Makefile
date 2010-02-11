SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPINC = -I. -I../common 
INCLUDE = $(DEPINC) -I../libluabind -I/usr/include/lua5.1
DEFS = 

all: mediator

include ../Make.conf

install: mediator
	$(INSTALL) mediator $(PREFIX)/sbin

mediator: $(OBJS)
	$(CXX) -o $(@) $(OBJS) \
    -Wl,--whole-archive ../common/libaacommon.a -Wl,-no-whole-archive \
	../slirp/libslirp.a \
	../libluabind/libluabind.a \
	-lpqxx -lsqlite3 -llua5.1 -lz \
	$(THREAD_LIBS) 

clean:
	$(RM) $(OBJS) mediator 

.PHONY: test
test:
	$(MAKE) -C test test

distclean: clean

# DO NOT DELETE

control_packet_processor.o: control_packet_processor.h packet_processor.h
control_packet_processor.o: event_handler.h mpacket.h switch.h net_typedefs.h
control_packet_processor.o: ../common/mac.h node.h ../common/metric.h
control_packet_processor.o: ../common/timing_message.h stats.h log.h
data_packet_processor.o: data_packet_processor.h packet_processor.h
data_packet_processor.o: event_handler.h mpacket.h switch.h net_typedefs.h
data_packet_processor.o: ../common/mac.h node.h ../common/metric.h
data_packet_processor.o: ../common/timing_message.h stats.h log.h
med_lua_export.o: ../common/lua.hpp
mediator.o: ../common/liboptions.h ../common/kill.h ../common/static_set.h
mediator.o: ../common/lua_register.h ../common/lua.hpp ../common/units.h
mediator.o: ../common/cast.h ../common/uint64_t_hash.h ../common/parameters.h
mediator.o: ../common/param_factory.h ../common/options.h mediator_server.h
mediator.o: reactor.h event_handler.h switch.h net_typedefs.h ../common/mac.h
mediator.o: node.h mpacket.h ../common/metric.h stats.h packet_acceptor.h
mediator.o: control_packet_processor.h packet_processor.h
mediator.o: ../common/timing_message.h data_packet_processor.h slirp_acceptor.h
mediator.o: vde_acceptor.h scheduler.h net_timing_model.h
mediator_server.o: mediator_server.h reactor.h event_handler.h switch.h
mediator_server.o: net_typedefs.h ../common/mac.h node.h mpacket.h
mediator_server.o: ../common/metric.h stats.h packet_acceptor.h
mediator_server.o: control_packet_processor.h packet_processor.h
mediator_server.o: ../common/timing_message.h data_packet_processor.h
mediator_server.o: slirp_acceptor.h vde_acceptor.h scheduler.h
net_constant.o: net_timing_model.h net_typedefs.h ../common/mac.h
net_constant.o: ../common/metric.h ../common/param_factory.h
net_constant.o: ../common/parameters.h ../common/cast.h ../common/units.h
net_constant.o: ../common/lua.hpp ../common/kill.h
net_dest_based.o: net_timing_model.h net_typedefs.h ../common/mac.h
net_dest_based.o: ../common/metric.h ../common/param_factory.h
net_dest_based.o: ../common/parameters.h ../common/cast.h ../common/units.h
net_dest_based.o: ../common/lua.hpp ../common/kill.h
net_new_dest_based.o: net_timing_model.h net_typedefs.h ../common/mac.h
net_new_dest_based.o: ../common/metric.h ../common/param_factory.h
net_new_dest_based.o: ../common/parameters.h ../common/cast.h ../common/units.h
net_new_dest_based.o: ../common/lua.hpp ../common/kill.h
net_simple_switch.o: net_timing_model.h net_typedefs.h ../common/mac.h
net_simple_switch.o: ../common/metric.h ../common/liboptions.h ../common/kill.h
net_simple_switch.o: ../common/static_set.h ../common/lua_register.h
net_simple_switch.o: ../common/lua.hpp ../common/units.h ../common/cast.h
net_simple_switch.o: ../common/uint64_t_hash.h ../common/parameters.h
net_simple_switch.o: ../common/param_factory.h ../common/options.h
net_timing_model.o: net_timing_model.h net_typedefs.h ../common/mac.h
net_timing_model.o: ../common/metric.h ../common/kill.h
node.o: node.h net_typedefs.h ../common/mac.h stats.h
packet_acceptor.o: reactor.h event_handler.h packet_processor.h mpacket.h
packet_acceptor.o: switch.h net_typedefs.h ../common/mac.h node.h
packet_acceptor.o: ../common/metric.h ../common/timing_message.h
packet_acceptor.o: packet_acceptor.h
packet_processor.o: packet_processor.h event_handler.h mpacket.h switch.h
packet_processor.o: net_typedefs.h ../common/mac.h node.h ../common/metric.h
packet_processor.o: ../common/timing_message.h
reactor.o: reactor.h event_handler.h stats.h
scheduler.o: scheduler.h net_typedefs.h ../common/mac.h mpacket.h node.h
scheduler.o: switch.h ../common/metric.h stats.h log.h
slirp_acceptor.o: reactor.h event_handler.h packet_processor.h mpacket.h
slirp_acceptor.o: switch.h net_typedefs.h ../common/mac.h node.h
slirp_acceptor.o: ../common/metric.h ../common/timing_message.h
slirp_acceptor.o: slirp_acceptor.h ../slirp/libslirp.h
stats.o: stats.h
switch.o: switch.h net_typedefs.h ../common/mac.h node.h mpacket.h
switch.o: ../common/metric.h data_packet_processor.h packet_processor.h
switch.o: event_handler.h ../common/timing_message.h scheduler.h stats.h
switch.o: net_timing_model.h log.h ../common/dump_gzip.h ../common/liboptions.h
switch.o: ../common/kill.h ../common/static_set.h ../common/lua_register.h
switch.o: ../common/lua.hpp ../common/units.h ../common/cast.h
switch.o: ../common/uint64_t_hash.h ../common/parameters.h
switch.o: ../common/param_factory.h ../common/options.h ../common/libmetric.h
switch.o: ../common/tagged_metric.h ../common/heartbeat.h
switch.o: ../common/aggregator.h
vde_acceptor.o: reactor.h event_handler.h packet_processor.h mpacket.h switch.h
vde_acceptor.o: net_typedefs.h ../common/mac.h node.h ../common/metric.h
vde_acceptor.o: ../common/timing_message.h vde_acceptor.h
