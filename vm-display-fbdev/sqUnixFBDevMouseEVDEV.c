/* sqUnixFBDevMouseEVDEV.c -- Use libevdev to read /dev/input/eventX mouse
 * 
 * Author: Ken.Dickey@whidbey.com
 * 
 * Last edited: 22 December 2019
 */

/*
 * Copyright (C) 2019 Kenneth A Dickey
 * All Rights Reserved.
 * 
 * This file is part of Unix Squeak.
 * 
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 * 
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * 
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *   DEALINGS IN THE SOFTWARE.
 */

/*==================*/
/* Mouse Point      */
/*==================*/

/*======FYI
typedef struct
{
  int x, y;
} SqPoint;
 ======*/

static int wheelDelta = 0;  /* Note: also reset in clearMouseButtons() */

SqPoint mousePosition= { 0, 0 };	/* position at last motion event */
int	swapBtn= 0;			/* 1 to swap yellow and blue buttons */

void setMousePosition( int newX, int newY ) {
  mousePosition.x = newX;
  mousePosition.y = newY;
}

void copyMousePositionInto(SqPoint *mousePt) {
  mousePt->x = mousePosition.x;
  mousePt->y = mousePosition.y;
}

void updateMousePosition(struct input_event* evt) {
/* Note: As with Squeak Disply:  {0,0} at topLeft of screen 
	down  => delta-Y DOWN  is positive
	right => delta-X RIGHT is positive  
*/
  if (evt->type == EV_REL) {
    switch (evt->code) {
      case REL_X:
	/* no less than 0 */
	mousePosition.x = max(0, mousePosition.x + evt->value) ;
	break;
      case REL_Y:
	/* no less than 0 */
	mousePosition.y = max(0, mousePosition.y + evt->value) ; 
	break;
      case REL_WHEEL:
	wheelDelta += evt->value;
/*	printf( "Wheel VALUE: %d; DELTA: %d ", mouseWheelDelta(), evt->value ) ; */
	break;
      default:
	break;
    }
  }
}

void printMouseState() {
   if ( (mousePosition.x != 0) || (mousePosition.y != 0) ) {
     printf( "Mouse at %4d,%4d ", mousePosition.x, mousePosition.y );
     printButtons(   mouseButtons() );
     printModifiers( modifierState() ); /* sqUnixFBDevKeyboardEVDEV.c */
     if (mouseWheelDelta() != 0) {
       	printf( " Mouse Wheel: %d", mouseWheelDelta() );
     }
     printf("\n");
   }
}

void printButtons(int buttons)
{
  if (buttons & RedButtonBit)    printf(" Left/Red");
  if (buttons & YellowButtonBit) printf(" Mid/Yellow");
  if (buttons & BlueButtonBit)   printf(" Right/Blue");
}

void clearMouseWheel() {  wheelDelta = 0 ; }

int mouseWheelDelta() { return ( wheelDelta ) ; }


/*==================*/
/* Mouse buttons    */
/*==================*/

int mouseButtonsDown = 0;  /* (left|mid|right) = (Red|Yellow|Blue) */

int mouseButtons() { return ( mouseButtonsDown ) ; }
int buttonState()  { return ( mouseButtonsDown ) ; } /* DEPRICATED */

void clearMouseButtons() { mouseButtonsDown = 0 ; wheelDelta = 0; }

void updateMouseButtons(struct input_event* evt) {
  if (evt->type == EV_KEY) {
    if (evt->value == 1) { /* button down */
      switch (evt->code) {
	case BTN_LEFT:   mouseButtonsDown |= LeftMouseButtonBit;  break;
	case BTN_MIDDLE: mouseButtonsDown |= MidMouseButtonBit;   break;
	case BTN_RIGHT:  mouseButtonsDown |= RightMouseButtonBit; break;
	default: break;
      }
    } else if (evt->value == 0) { /* button up */
      switch (evt->code) {
	case BTN_LEFT:   mouseButtonsDown &= ~LeftMouseButtonBit;  break;
	case BTN_MIDDLE: mouseButtonsDown &= ~MidMouseButtonBit;   break;
	case BTN_RIGHT:  mouseButtonsDown &= ~RightMouseButtonBit; break;
	default: break;
      }
    /* ignore repeats (evt->value == 2) */
    }
  }
}



/** @@marker@@ **/


static void ms_ps2_handleEvents(_self)
{
  unsigned char buf[3*8];
  int		n;
  
  if ((n= ms_read(self, buf, sizeof(buf), 3, PS2_READ_DELAY)) >= 3)
    {
      unsigned char *cmd= buf;
      while (n >= 3)
	{
	  int b= 0, dx, dy;
	  // The protocol requires the top 2 bits clear and bit 3 set.
	  // Some Micro$oft mice violate this, but any luser stupid
	  // enough to buy a M$ mouse deserves what they get.
	  if (0x08 != (cmd[0] & 0xc8))
	    {
	      fprintf(stderr, "%s: illegal command: %02x %02x %02x\n", self->msName,
		      cmd[0], cmd[1], cmd[2]);
	      ms_ps2_flush(self);	// resync the stream
	      return;
	    }
	  if (cmd[0] & 1) b |= RedButtonBit;
	  if (cmd[0] & 2) b |= BlueButtonBit;
	  if (cmd[0] & 4) b |= YellowButtonBit;
	  dx= cmd[1];  if (cmd[0] & 0x10) dx -= 256;
	  dy= cmd[2];  if (cmd[0] & 0x20) dy -= 256;
	  dy= -dy;
	  self->callback(b, dx, dy);
	  n -= 3;  cmd += 3;
	}
    }
}


static int ms_ps2_send(_self, unsigned char *command, int len)
{
  unsigned char buf[1];
  int i;
  DPRINTF("%s: send\n", self->msName);
  for (i= 0;  i < len;  ++i)
    {
    resend:
      if (1 != write(self->fd, command + i, 1))
	{
	  fprintf(stderr, "%s: could not write command to ps2\n", self->msName);
	  return 0;
	}
      DPRINTF(">%02x\n", command[i]);
      if (1 != ms_read(self, buf, 1, 1, PS2_SEND_DELAY))
	{
	  DPRINTF("%s: send failed\n", self->msName);
	  return 0;
	}
      switch (buf[0])
	{
	case PS2_OK:
	case PS2_SELFTEST_OK:	 /* /dev/input/mice emulation is broken */
	  break;
	case PS2_ERROR:
	  fprintf(stderr, "%s: error response in send\n", self->msName);
	  return 0;
	case PS2_RESEND:
	  DPRINTF("%s: resend\n", self->msName);
	  goto resend;
	default:
	  fprintf(stderr, "%s: illegal response %02x in send\n", self->msName, buf[0]);
	  break;
	}
    }
  return 1;
}


static void ms_ps2_disable(_self)
{ 
  unsigned char command[]= { PS2_DISABLE };
  DPRINTF("%s: disable\n", self->msName);
  if (1 != write(self->fd, command, 1))
    {
	fprintf(stderr, "%s: could not write command to ps2\n", self->msName);
	return;
    }
  DPRINTF(">%02x\n", command[0]);
  while (1 == ms_read(self, command, 1, 1, PS2_DISABLE_DELAY))
    if (PS2_OK == command[0])
      break;
}


static int ms_ps2_enable(_self)
{
  unsigned char command[]= { PS2_ENABLE };
  DPRINTF("%s: enable\n", self->msName);
  return ms_ps2_send(self, command, sizeof(command));
}


static int ms_ps2_reset(_self)
{
  unsigned char command[]= { PS2_RESET }, buf[2];
  DPRINTF("%s: reset\n", self->msName);
  if (!ms_ps2_send(self, command, sizeof(command)))
    return -1;
  if (2 == ms_read(self, buf, 2, 2, PS2_RESET_DELAY))
    {
      DPRINTF("%s: response %02x %02x\n", self->msName, buf[0], buf[1]);
      switch (buf[0])
	{
	case PS2_SELFTEST_OK:
	  return buf[1];	// mouse device id
	  break;
	case PS2_ERROR:
	  fprintf(stderr, "%s: self-test failed\n", self->msName);
	  break;
	default:
	  DPRINTF("%s: bad response\n", self->msName);
	  break;
	}
    }
  /* /dev/input/mice emulation returns PS2_SELFTEST_OK where send()
     expects PS2_OK, causing control to fall through to here.  we pick
     up the mouse id immediately in the flush(), so the only harm done
     is a misleading "reset failed" message while debugging.  */
  ms_ps2_flush(self);
  DPRINTF("%s: reset failed\n", self->msName);
  return -1;
}


static void ms_ps2_init(_self)
{
  int id;
  ms_ps2_disable(self);
  id= ms_ps2_reset(self);
  DPRINTF("%s: mouse id %02x\n", self->msName, id);
  ms_ps2_enable(self);
}



static sqInt getButtonState(void)
{
  /* red button honours the modifiers:
   *	red+ctrl    = yellow button
   *	red+command = blue button
   */
  int buttons   = buttonState();
  int modifiers = modifierState();
#ifdef DEBUG_MOUSE
  printf( "BUTTONS (getButtonState)" );
  printModifiers( modifiers );
  printButtons( buttons );
  printf( "\n" );
#endif
  return buttons | (modifiers << 3); 
}


static void recordMouseEvent(void)
{
  int state= getButtonState();
  sqMouseEvent *evt= allocateMouseEvent();
  evt->x= mousePosition.x;
  evt->y= mousePosition.y;
  evt->buttons= (state & 0x7);
  evt->modifiers= (state >> 3);
  evt->nrClicks= 0; /*@@FIXME: Mouse Wheel?? @@*/
/*@@@  signalInputEvent(); @@@*/
#if DEBUG_MOUSE_EVENTS
  printf( "EVENT (recordMouseEvent): time: %ld  mouse (%d,%d)", evt->timeStamp, mousePosition.x, mousePosition.y );
  printModifiers( state >> 3 );
  printButtons( state & 7 );
  printf( "\n" );
#endif
}

static void recordMouseWheelEvent(int dx, int dy)
{
  sqMouseEvent *evt= allocateMouseWheelEvent();
  evt->x= dx;
  evt->y= dy;
  // VM reads fifth (4th 0-based) field for event's modifiers
  evt->buttons= (getButtonState() >> 3);
  signalInputEvent();
#if DEBUG_MOUSE_EVENTS
  printf("EVENT (recordMouseWheelEvent): time: %ld  mouse dx %d dy %d", evt->timeStamp, dx, dy);
  printButtons(evt->buttons);
  printf("\n");
#endif
}


/*		--- E O F ---			*/
