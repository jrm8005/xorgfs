#include "xcalls.h"

#ifdef DEBUG
#include <stdio.h>
#include <stdarg.h>

/* So, anyone want to tell me why FUSE won't let me do this?
 * I can't printf(), either
 */
void debugmsg(const char *fmt, ...)
{
	FILE *fd;
	fd = fopen("/tmp/xorgfs.log", "a+");
	va_list ap;

	va_start(ap, fmt);

	if (fd) {
		vfprintf(fd, fmt, ap);
		fclose(fd);
	}

	va_end(ap);
}

#else

/* Hopefully GCC makes this a NOP */
void debugmsg(const char *fmt, ...)
{
	;
}

#endif

/* Sets x and y to mouse cursor position */
int xorg_get_cursor_pos(int *x, int *y)
{
	Display *dsp = XOpenDisplay(NULL);
	XButtonEvent event;
	
	if (!dsp)
		return 1;

	XQueryPointer(dsp, RootWindow(dsp, DefaultScreen(dsp)),
			&event.root, &event.window,
			&event.x_root, &event.y_root,
			&event.x, &event.y,
			&event.state);

	*x = event.x;
	*y = event.y;

	XCloseDisplay(dsp);

	return 0;
}

/* Returns contents of X Selection Buffer
 *
 * This code was gratuitously stoled from xclip and modified a wee bit
 * 
 * n is the size of the clipboard contents
 * 
 * NOTE: user must free this!!
 */
char *xorg_get_buffer(unsigned long *n)
{
	unsigned char *sel_buf;		/* buffer for selection data */
	unsigned long sel_len = 0;	/* length of sel_buf */
	unsigned long sel_all = 0;	/* allocated size of sel_buf */
	Window win;			/* Window */
	XEvent evt;			/* X Event Structures */
	unsigned int context =XCLIB_XCOUT_NONE;
	Atom sseln = XA_PRIMARY;

	Display *dpy = XOpenDisplay(NULL);
	win = XCreateSimpleWindow(
		dpy,
		DefaultRootWindow(dpy),
		0,
		0,
		1,
		1,
		0,
		0,
		0
	);

	XSelectInput(dpy, win, PropertyChangeMask);

	while (1)
	{
		/* only get an event if xcout() is doing something */
		if (context != XCLIB_XCOUT_NONE)
			XNextEvent(dpy, &evt);

		/* fetch the selection, or part of it */
		xcout(
				dpy,
				win,
				evt,
				sseln,
				&sel_buf,
				&sel_len,
				&context
		     );

		/* only continue if xcout() is doing something */
		if (context == XCLIB_XCOUT_NONE)
			break;
	}

	if (sel_len == 0) {
		sel_buf = NULL;
	}

	*n = sel_len;

	XCloseDisplay(dpy);

	return sel_buf;
}

/* Begin theft: this code was also gratuitously stolen from xclip */
int xcout (
	Display *dpy,
	Window win,
	XEvent evt,
	Atom sel,
	unsigned char **txt,
	unsigned long *len,
	unsigned int *context
)
{
	/* a property for other windows to put their selection into */
	Atom pty, inc, pty_type;
	int pty_format;
		
	/* buffer for XGetWindowProperty to dump data into */
	unsigned char *buffer;
	unsigned long pty_size, pty_items;

	/* local buffer of text to return */
	unsigned char *ltxt;

	pty = XInternAtom(dpy, "XCLIP_OUT", False);

	switch (*context)
	{
		/* there is no context, do an XConvertSelection() */
		case XCLIB_XCOUT_NONE:
			/* initialise return length to 0 */
			if (*len > 0)
			{
				free(*txt);
				*len = 0;
			}

			/* send a selection request */
			XConvertSelection(
				dpy,
				sel,
				XA_STRING,
				pty,
				win,
				CurrentTime
			);
			*context = XCLIB_XCOUT_SENTCONVSEL;
			return(0);
		
		case XCLIB_XCOUT_SENTCONVSEL:
			inc = XInternAtom(dpy, "INCR", False);

			if (evt.type != SelectionNotify)
				return(0);

			/* find the size and format of the data in property */
			XGetWindowProperty(
				dpy,
				win,
				pty,
				0,
				0,
				False,
				AnyPropertyType,
				&pty_type,
				&pty_format,
				&pty_items,
				&pty_size,
				&buffer
			);
			XFree(buffer);

			if (pty_type == inc)
			{
				/* start INCR mechanism by deleting property */
				XDeleteProperty(dpy, win, pty);
				XFlush(dpy);
				*context = XCLIB_XCOUT_INCR;
				return(0);
			}

			/* if it's not incr, and not format == 8, then there's
			 * nothing in the selection (that xclip understands,
			 * anyway)
			 */ 
			if (pty_format != 8)
			{
				*context = XCLIB_XCOUT_NONE;
				return(0);
			}

			/* not using INCR mechanism, just read the property */
			XGetWindowProperty(
				dpy,
				win,
				pty,
				0,
				(long)pty_size,
				False,
				AnyPropertyType,
				&pty_type,
				&pty_format,
				&pty_items,
				&pty_size,
				&buffer
			);
	
			/* finished with property, delete it */
			XDeleteProperty(dpy, win, pty);
		
			/* copy the buffer to the pointer for returned data */
			ltxt = (unsigned char *)malloc(pty_items);
			memcpy(ltxt, buffer, pty_items);

			/* set the length of the returned data */
			*len = pty_items;
			*txt = ltxt;

			/* free the buffer */
			XFree(buffer);
			
			*context = XCLIB_XCOUT_NONE;

			/* complete contents of selection fetched, return 1 */
			return(1);
	
		case XCLIB_XCOUT_INCR:
			/* To use the INCR method, we basically delete the
			 * property with the selection in it, wait for an
			 * event indicating that the property has been created,
			 * then read it, delete it, etc.
			 */

			/* make sure that the event is relevant */
			if (evt.type != PropertyNotify)
				return(0);

			/* skip unless the property has a new value */
			if (evt.xproperty.state != PropertyNewValue)
				return(0);
	
			/* check size and format of the property */
			XGetWindowProperty(
				dpy,
				win,
				pty,
				0,
				0,
				False,
				AnyPropertyType,
				&pty_type,
				&pty_format,
				&pty_items,
				&pty_size,
				(unsigned char **)&buffer
			);

			if (pty_format != 8)
			{
				/* property does not contain text, delete it
				 * to tell the other X client that we have read
				 * it and to send the next property
				 */
				XFree(buffer);
				XDeleteProperty(dpy, win, pty);
				return(0);
			}

			if (pty_size == 0)
			{
				/* no more data, exit from loop */
				XFree(buffer);
				XDeleteProperty(dpy, win, pty);
				*context = XCLIB_XCOUT_NONE;
				
				/* this means that an INCR transfer is now
				 * complete, return 1
				 */
				return(1);
			}

			/* if we have come this far, the propery contains
			 * text, we know the size.
			 */
			XGetWindowProperty(
				dpy,
				win,
				pty,
				0,
				(long)pty_size,
				False,
				AnyPropertyType,
				&pty_type,
				&pty_format,
				&pty_items,
				&pty_size,
				(unsigned char **)&buffer
			);
			
			/* allocate memory to ammodate data in *txt */
			if (*len == 0)
			{
				*len = pty_items;
				ltxt = (unsigned char *)malloc(*len);
			} else
			{
				*len += pty_items;
				ltxt = (unsigned char *)realloc(ltxt, *len);
			}

			/* add data to ltxt */
			memcpy(
				&ltxt[*len - pty_items],
				buffer,
				pty_items
			);

			*txt = ltxt;
			
			/* delete property to get the next item */
			XDeleteProperty(dpy, win, pty);
			XFlush(dpy);
			return(0);
	}

	return (0);
}
/* End theft */
