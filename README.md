# FBDevVM

vm-display-fbdev using libevdev rather than ps2

This is working on aarch64 RasPi3 and RasPi4 Alpine Linux (aarch64;MUSL;busybox)
as well as LePotato (aarch64;libc;AML-S905X-CC) Armbian [Debian Buster] and 
on amd64 under Alpine Linux.

Both interpreter and Cog were tested with both Squeak and Cuis images.
RasPi3 seems to support only 16 bit framebuffer depth; RasPi4 both 16 and 32 bit depths

Full code now in the OpenSmalltalk repository:
  https://github.com/OpenSmalltalk/opensmalltalk-vm

See notes in:
  build.linux64ARMv8/HowToBuild -- MUSL at end
  platforms/unix/vm-display-fbdev/AlpineLinux-Notes.txt
  platforms/unix/vm-display-fbdev/Armian-Notes.txt

Note:
  MUSL requires flags "-DMUSL -D_GNU_SOURCE" ..
	and read build.linux64ARMv8/HowToBuild
  LibEvdev requires: "-DUSEEVDEV"

e.g. in 'mvm':
 OPT="-g -O2 -DNDEBUG -DDEBUGVM=0 -DMUSL -D_GNU_SOURCE -DUSEEVDEV"

remove "--without-vm-display-fbdev"

So go look at and use:
    https://github.com/OpenSmalltalk/opensmalltalk-vm
