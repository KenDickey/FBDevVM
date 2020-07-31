/* sqUnixFBDevEVDEV.h -- libevdev code common to mouse and keyboard
 *
 * Author: Ken.Dickey@whidbey.com
 * Last Updated: August 2020
 */

#ifndef SQUEAK_LIBEVDEV
#define SQUEAK_LIBEVDEV

/* You need to define  _GNU_SOURCE  for MUSL  */

#include "sq.h"
#include <linux/input.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <signal.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h> /* PATH_MAX */

#include "sqAssert.h"
#include "sqMemoryAccess.h"
char* sqMemoryBase = (char *)0;
sqInt success(sqInt whatever) { return( SUCCESS ) ; } /* @@@???@@@ */
#include "sqaio.h"
#include "sqUnixCharConv.h"
#include "sqSCCSVersion.h"
#include "sqUnixMain.h"
#include "debug.h"

#if defined(ioMSecs)
# undef ioMSecs
#endif
long ioMSecs(void) { return ( 0 ) ; }


#ifndef input_event_sec
#define input_event_sec time.tv_sec
#define input_event_usec time.tv_usec
#endif

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

#define DEV_INPUT_EVENT "/dev/input"
#define EVENT_DEV_NAME "event"

#ifndef EV_SYN
#define EV_SYN 0
#endif
#ifndef SYN_MAX
#define SYN_MAX 3
#define SYN_CNT (SYN_MAX + 1)
#endif
#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT 2
#endif
#ifndef SYN_DROPPED
#define SYN_DROPPED 3
#endif

/* wiki.squeak.org/squeak/897 shows:
   red    #b100 left  (4)	button1
   yellow #b010 mid   (2)	button2
   blue   #b001 right (1)	button3
 So binary view of bit matches mouse (l|m|r) position
 */
#define LeftMouseButtonBit  RedButtonBit
#define MidMouseButtonBit   YellowButtonBit
#define RightMouseButtonBit BlueButtonBit


