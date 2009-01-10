#include "xcalls.h"

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
