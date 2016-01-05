all: build

include ../src/Make.conf 


#Uncomment only the entry you choose
#Leave commented if you prefer default configuration
#RAM=128M
#RAM=256M
#RAM=512M
#RAM=1G
#RAM=2G
#RAM=4G
#RAM=8G
#RAM=16G

DIST=karmic
HDD=$(DIST)64.img

RBSDS=$(shell ls *-reset.bsd)
BSDS=$(patsubst %-reset.bsd,%.bsd,$(RBSDS))

build: $(BSDS) $(HDD)

clean:
	$(RM) $(BSDS) *-boot

distclean: clean
	$(RM) $(HDD)

$(HDD):
	./vmbuild $(DIST) $(HDD)

lzma: $(HDD)
	lzma -c $(HDD) > $(HDD).lzma

# sudo mount -o loop,offset=32256,ro $(HDD) tmp 

%.bsd: %-reset.bsd $(HDD)
	if [ -f tf.rom ]  && [! -f $(SIMNOW_DIR)/Images/tf.rom ]; then \
	    $(CP) tf.rom $(SIMNOW_DIR)/Images; \
	fi; \
	if [ -z "${RAM}" ] ; then \
		./initbsd $(SIMNOW_DIR) $* $(HDD)  2>&1 |tee initbsd.out; \
	else \
		./initbsd $(SIMNOW_DIR) $*  $(HDD) $(RAM) 2>&1 |tee initbsd.out; \
	fi;
	@echo "#### Note: don't worry if you see a 'segmentation fault' just above"
	a=`tail -1 /home/giorgi/C5test1/cotson-726/trunk/data/initbsd.out| grep -i 'segmentation fault'`; \
	if [ "$$a" != "" ]; then exit 0; fi

mount:
	@echo "##### MOUNT disk to tmp"; \
	mkdir tmp; \
	sudo mount -o loop,offset=16384,rw $(HDD) tmp; \
	echo "##### ENTERING A NEW SHELL (cd tmp)"; \
	bash; \
	echo "##### EXIT THE SHELL AND UMOUNT"; \
	sudo umount tmp; \
	rmdir tmp

