SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
INCLUDE = -I. -I../common -I../libluabind -I/usr/include/lua5.1
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

distclean: clean

# DO NOT DELETE

control_packet_processor.o: control_packet_processor.h packet_processor.h
control_packet_processor.o: event_handler.h mpacket.h switch.h net_typedefs.h
control_packet_processor.o: node.h stats.h log.h
data_packet_processor.o: data_packet_processor.h packet_processor.h
data_packet_processor.o: event_handler.h mpacket.h switch.h net_typedefs.h
data_packet_processor.o: node.h stats.h log.h
mediator.o: mediator_server.h reactor.h event_handler.h switch.h net_typedefs.h
mediator.o: node.h mpacket.h stats.h packet_acceptor.h
mediator.o: control_packet_processor.h packet_processor.h
mediator.o: data_packet_processor.h slirp_acceptor.h vde_acceptor.h scheduler.h
mediator.o: net_timing_model.h
mediator_server.o: mediator_server.h reactor.h event_handler.h switch.h
mediator_server.o: net_typedefs.h node.h mpacket.h stats.h packet_acceptor.h
mediator_server.o: control_packet_processor.h packet_processor.h
mediator_server.o: data_packet_processor.h slirp_acceptor.h vde_acceptor.h
mediator_server.o: scheduler.h
net_constant.o: net_timing_model.h net_typedefs.h
net_dest_based.o: net_timing_model.h net_typedefs.h
net_new_dest_based.o: net_timing_model.h net_typedefs.h
net_simple_switch.o: net_timing_model.h net_typedefs.h
net_timing_model.o: net_timing_model.h net_typedefs.h
node.o: node.h net_typedefs.h stats.h
packet_acceptor.o: reactor.h event_handler.h packet_processor.h mpacket.h
packet_acceptor.o: switch.h net_typedefs.h node.h packet_acceptor.h
packet_processor.o: packet_processor.h event_handler.h mpacket.h switch.h
packet_processor.o: net_typedefs.h node.h
reactor.o: reactor.h event_handler.h stats.h
scheduler.o: scheduler.h net_typedefs.h mpacket.h node.h switch.h stats.h log.h
slirp_acceptor.o: reactor.h event_handler.h packet_processor.h mpacket.h
slirp_acceptor.o: switch.h net_typedefs.h node.h slirp_acceptor.h
slirp_acceptor.o: ../slirp/libslirp.h
stats.o: stats.h
switch.o: switch.h net_typedefs.h node.h mpacket.h data_packet_processor.h
switch.o: packet_processor.h event_handler.h scheduler.h stats.h
switch.o: net_timing_model.h log.h
vde_acceptor.o: reactor.h event_handler.h packet_processor.h mpacket.h switch.h
vde_acceptor.o: net_typedefs.h node.h vde_acceptor.h
