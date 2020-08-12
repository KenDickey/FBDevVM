# FBDevVM
vm-display-fbdev using libevdev rather than ps2

Currently, the engine is in pieces, spread out on the floor.  sqEVTest.c is only working

RasPi3 Alpine Linux (aarch64;MUSL;busybox) -- display + keystrokes & mouse tracking; NO mouse buttons yet.
mvm:
 OPT="-g -O2 -DNDEBUG -DDEBUGVM=0 -DMUSL -D_GNU_SOURCE -DUSEEVDEV"

