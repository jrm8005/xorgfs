#include "xcalls.h"

int xorg_get_cursor_pos(int *x, int *y)
{
	Display *dsp = XOpenDisplay(NULL);
	XEvent event;
	
	if (!dsp)
		return 1;

	XQueryPointer(dsp, RootWindow(dsp, DefaultScreen(dsp)),
			&event.xbutton.root, &event.xbutton.window,
			&event.xbutton.x_root, &event.xbutton.y_root,
			&event.xbutton.x, &event.xbutton.y,
			&event.xbutton.state);

	*x = event.xbutton.x;
	*y = event.xbutton.y;

	XCloseDisplay(dsp);

	return 0;
}
