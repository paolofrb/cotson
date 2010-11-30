all: mcpat

MCPAT = mcpat0.8_r274
include ../Make.conf

mcpat: $(MCPAT)
	$(MAKE) -C $(MCPAT) OPT="-O3 -msse2 -mfpmath=sse -DNTHREADS=4 -Icacti" opt; \
	$(CP) $(MCPAT)/mcpat .

$(MCPAT): $(MCPAT).tar.gz
	$(RM) $(MCPAT); tar xfz $(MCPAT).tar.gz

$(MCPAT).tar.gz:
	wget http://www.hpl.hp.com/research/mcpat/$(MCPAT).tar.gz

install: mcpat cotson2mcpat
	$(INSTALL) mcpat $(PREFIX)/sbin
	$(CP) cotson2mcpat $(PREFIX)/sbin

clean:
	$(RM) mcpat mcpat.db mcpat.xml mcpat.xml.txt testmcpat*; \
	$(MAKE) -C $(MCPAT) clean

distclean: clean
	$(RM) $(MCPAT).tar.gz $(MCPAT)

test: mcpat cotson2mcpat
	$(RM) mcpat.db mcpat.xml; \
	../../bin/cotson DBFILE=\"`pwd`/mcpat.db\" mcpat.lua ; \
	./cotson2mcpat file:mcpat.db 1 mcpat.xml -r -v ; \
	less mcpat.xml.txt
