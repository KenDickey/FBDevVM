# FBDevVM
vm-display-fbdev using libevdev rather than ps2

Basics are working on aarch64 RasPi3 Alpine Linux (aarch64;MUSL;busybox)

Both interpreter and Cog were tested.

Full code in repository:
  https://github.com/KenDickey/opensmalltalk-vm

Note:
  MUSL requires flags "-DMUSL -D_GNU_SOURCE" ..
	and read build.linux64ARMv8/HowToBuild
  LibEvdev requires: "-DUSEEVDEV"

mvm:
 OPT="-g -O2 -DNDEBUG -DDEBUGVM=0 -DMUSL -D_GNU_SOURCE -DUSEEVDEV"
 remove "--without-vm-display-fbdev"
