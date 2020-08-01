#!/bin/bash
export LSB_FIRST=1
export TARGET_ARCH="-march=armv8-a" 
# for MUSL:  -DMUSL -D_GNU_SOURCE
export CFLAGS=" -DMUSL -D_GNU_SOURCE -D__ARM_ARCH_ISA_A64 -DARM64 -D__arm__ -D__arm64__ -D__aarch64__"
export INCLUDES="-I/home/kend/OpenSmalltalk/oscogvm/src/vm -I/home/kend/OpenSmalltalk/oscogvm/platforms/Cross/vm -I/home/kend/OpenSmalltalk/oscogvm/platforms/unix/vm -I/home/kend/OpenSmalltalk/oscogvm/build.linux64ARMv8/squeak.stack.spur/build "
gcc -o evtest $TARGET_ARCH $CFLAGS $INCLUDES sqEVTest.c
echo "[Hopefully] compiled evtest"
