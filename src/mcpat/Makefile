all: mcpat

MCPAT = mcpat0.8_r274
include ../Make.conf
LIBSTDCPP32 = $(shell ls /usr/lib32/libstdc++.so.* | tail -n 1)

mcpat: $(MCPAT)
	$(MAKE) -C $(MCPAT) OPT="-m32 -O3 -msse2 -mfpmath=sse -DNTHREADS=4 -Icacti -L ." opt; \
	$(CP) $(MCPAT)/mcpat .

$(MCPAT): $(MCPAT).tar.gz
	$(RM) -rf $(MCPAT); mkdir $(MCPAT); tar --strip=1 -C $(MCPAT) -zxf $(MCPAT).tar.gz

$(MCPAT).tar.gz:
	wget http://www.hpl.hp.com/research/mcpat/$(MCPAT).tar.gz

install: mcpat cotson2mcpat
	$(INSTALL) mcpat $(PREFIX)/sbin
	$(CP) cotson2mcpat $(PREFIX)/sbin

clean:
	if [[ -d $(MCPAT) ]]; then \
	    $(MAKE) -C $(MCPAT) clean; \
	fi; \
	$(RM) $(MCPAT) mcpat mcpat.db mcpat.xml mcpat.xml.txt testmcpat* libstdc++.so


distclean: clean
	$(RM) $(MCPAT).tar.gz

test: mcpat cotson2mcpat
	$(RM) mcpat.db mcpat.xml; \
	../../bin/cotson DBFILE=\"`pwd`/mcpat.db\" mcpat.lua ; \
	./cotson2mcpat file:mcpat.db 1 mcpat.xml -r -v ; \
	less mcpat.xml.txt
