## 
## Copyright 1995 - 2008 ADVANCED MICRO DEVICES, INC.  All Rights Reserved. 
##

TARGETDIR := $(shell pwd)/../../..

# compiler and tools
CCC := g++
CC := gcc
LD := ld
AR := ar

# compiler flags
CXX := $(CCC)
CXXFLAGS += -Werror -Wall -fPIC 
CXXFLAGS += -I. -I../

all: copy

OBJS = monitor.o
LIB = libmonitor.so

.PHONY: clean copy dep

$(LIB): $(OBJS)
ifneq ($(shell basename $(LIB) .so),$(LIB))
	libtool --mode=link $(CXX) --shared -module $(OBJS) -o $(LIB)
else
	$(AR) rv $(LIB) $(OBJS)
endif

copy: $(LIB)
	cp -uf $(LIB) $(TARGETDIR)

clean:
	rm -f $(OBJS) $(LIB) 

dep:
	echo '# DO NOT DELETE THIS LINE -- make depend depends on it.' > .depend
	makedepend -f.depend *.cpp *.c 2> /dev/null
	rm -f .depend.bak

