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

decoder.o: decoder.h distorm_config.h wstring.h prefix.h textdefs.h x86defs.h
decoder.o: insts.h dis_instructions.h operands.h
dis_instructions.o: dis_instructions.h distorm_config.h decoder.h wstring.h
dis_instructions.o: prefix.h insts.h textdefs.h x86defs.h
distorm.o: distorm_config.h decoder.h wstring.h x86defs.h textdefs.h insts.h
distorm.o: dis_instructions.h prefix.h
insts.o: distorm_config.h insts.h dis_instructions.h decoder.h wstring.h
insts.o: prefix.h
operands.o: distorm_config.h operands.h decoder.h wstring.h prefix.h
operands.o: dis_instructions.h x86defs.h textdefs.h insts.h
prefix.o: prefix.h distorm_config.h decoder.h wstring.h textdefs.h x86defs.h
prefix.o: insts.h dis_instructions.h
textdefs.o: textdefs.h distorm_config.h wstring.h
wstring.o: wstring.h distorm_config.h
x86defs.o: x86defs.h distorm_config.h textdefs.h wstring.h insts.h
x86defs.o: dis_instructions.h decoder.h prefix.h
