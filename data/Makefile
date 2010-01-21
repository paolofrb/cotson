all: build

include ../src/Make.conf 

DIST=karmic
HDD=$(DIST)64.img

build: 1p.bsd 4p.bsd $(HDD)

clean:
	$(RM) *-boot $(HDD)

distclean: clean
	$(RM) 1p.bsd 4p.bsd $(DIST) $(HDD)

$(HDD):
	./vmbuild $(DIST) $(HDD)

%.bsd: %-reset.bsd $(HDD)
	./initbsd $(SIMNOW_DIR) $*; \
	echo "#### Note: don't worry if you see a 'segmentation fault' just above"

