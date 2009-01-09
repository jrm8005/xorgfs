#ifndef _XCALLS_H_INCLUDED
#define _XCALLS_H_INCLUDED

/* X11 stuff */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

int xorg_get_cursor_pos(int *x, int *y);

#endif /* _XCALLS_H_INCLUDED */
