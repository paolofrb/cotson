
Note, all these directions are for running on a relatively modern
Linux distribution with the standard build tools all installed.  For
reproducibility purposes if you care, we performed builds on both
SuSE 9.2 Pro 64-bit and Fedora Core 2 64-bit and they worked fine.

In principle this could also be done under Cygwin, but I haven't tried
so Caveat Emptor.

Creating the Emerald BIOS:

  --  Get the Plex86/Bochs/QEMU LGPL VGABios version 0.5a from Savannah
	(web site "http://savannah.nongnu.org/", search for "vgabios"),
	package file "vgabios-0.5a.tgz".
  --  Execute "tar -xzf vgabios-0.5a.tgz".
  --  CD into the "vgabios-0.5a" directory.
  --  Execute "patch -p1 < emerald_bios_src.patch".

You should now have the full source code of the Emerald BIOS.

Creating tool chain to build it:

  --  Make sure you have GCC installed.  A 3.X variant was used for
	the builds we have, but any will probably work, it just uses
	the C preprocessor.
  --  Get the as86/ld86 devtools AND the bcc compiler commonly used
	with them.  We used the 0.16.17 version, which was the most
	recent at this writing.  The Linux binary package file is
	"Dev86bin-0.16.17.tar.gz".
  --  If using the binary package, which we did, then untar them as
	root in the root directory, i.e. as root in /, execute
	"tar -xzf Dev86bin-0.16.17.tar.gz".

Building:

  --  CD into the changed "vgabios-0.5a" directory from the "Creating
	the Emerald BIOS" section above.
  --  Execute "make".
  --  Result will be "VGABIOS-lgpl-latest.bin".

