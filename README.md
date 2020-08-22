# FBDevVM

vm-display-fbdev using libevdev rather than ps2

Basics are working on aarch64 RasPi3 and RasPi4 Alpine Linux (aarch64;MUSL;busybox)
as well as LePotato (aarch64;AML-S905X-CC) Armbian [Debian Buster].

Both interpreter and Cog were tested with both Squeak and Cuis images.

RasPi3 seems to support only 16 bit framebuffer depth; RasPi4 both 16 and 32 bit depths

Full code in repository:
  https://github.com/KenDickey/opensmalltalk-vm

Note:
  MUSL requires flags "-DMUSL -D_GNU_SOURCE" ..
	and read build.linux64ARMv8/HowToBuild
  LibEvdev requires: "-DUSEEVDEV"

mvm:
 OPT="-g -O2 -DNDEBUG -DDEBUGVM=0 -DMUSL -D_GNU_SOURCE -DUSEEVDEV"

remove "--without-vm-display-fbdev"

See also: 
  'AlpineLinux-Notes.txt'
  'Armian-Notes.txt'
