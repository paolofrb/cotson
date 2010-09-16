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

lzma: $(HDD)
	lzma -c $(HDD) > $(HDD).lzma

# sudo mount -o loop,offset=32256,ro $(HDD) tmp 

%.bsd: %-reset.bsd $(HDD)
	if [[ -f tf.rom && ! -f $(SIMNOW_DIR)/Images/tf.rom ]]; then \
	    cp tf.rom $(SIMNOW_DIR)/Images; \
	fi; \
	./initbsd $(SIMNOW_DIR) $*; \
	echo "#### Note: don't worry if you see a 'segmentation fault' just above"

mount:
	@echo "##### MOUNT disk to tmp"; \
	mkdir tmp; \
	sudo mount -o loop,offset=16384,rw $(HDD) tmp; \
	echo "##### ENTERING A NEW SHELL (cd tmp)"; \
	bash; \
	echo "##### EXIT THE SHELL AND UMOUNT"; \
	sudo umount tmp; \
	rmdir tmp

