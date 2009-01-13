#ifndef _XCALLS_H_INCLUDED
#define _XCALLS_H_INCLUDED

/* X11 stuff */
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlibint.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string.h>

/* Begin theft: Gratuitously stoled from xclip */
/* xcout() contexts */
#define XCLIB_XCOUT_NONE	0	/* no context */
#define XCLIB_XCOUT_SENTCONVSEL	1	/* sent a request */
#define XCLIB_XCOUT_INCR	2	/* in an incr loop */
/* End theft */

int xorg_get_cursor_pos(int *x, int *y);
char *xorg_get_buffer(unsigned long *n);
int xcout (
	Display *dpy,
	Window win,
	XEvent evt,
	Atom sel,
	unsigned char **txt,
	unsigned long *len,
	unsigned int *context
);

#endif /* _XCALLS_H_INCLUDED */
