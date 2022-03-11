#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>

Display *display;   // Points to the X server
int screen;     // which screen of the display to use
Window win;     // the first window
GC gc;          // the graphical context (where you draw)

void close_x() {
/* it is good programming practice to return system resources to the 
   system...
*/
	XFreeGC(display, gc);
	XDestroyWindow(display,win);
	XCloseDisplay(display);	
	exit(0);				
}

void init_x() {
	/* get the colors black and white (see section for details) */
	unsigned long black,white;

	/* use the information from the environment variable DISPLAY 
	   to create the X connection:
	*/	
	display=XOpenDisplay((char *)0);
   	screen=DefaultScreen(display);
	black=BlackPixel(display,screen),	/* get color black */
	white=WhitePixel(display, screen);  /* get color white */

	/* once the display is initialized, create the window.
	   This window will be have be 200 pixels across and 300 down.
	   It will have the foreground white and background black
	*/
   	win=XCreateSimpleWindow(display,DefaultRootWindow(display),0,0,	
		200, 300, 5, white, black);

	/* here is where some properties of the window can be set.
	   The third and fourth items indicate the name which appears
	   at the top of the window and the name of the minimized window
	   respectively.
	*/
	XSetStandardProperties(display,win,"My Window","HI!",None,NULL,0,NULL);

	/* this routine determines which types of input are allowed in
	   the input.  see the appropriate section for details...
	*/
	XSelectInput(display, win, ExposureMask|ButtonPressMask|KeyPressMask);

	/* create the Graphics Context */
        gc=XCreateGC(display, win, 0,0);        

	/* here is another routine to set the foreground and background
	   colors _currently_ in use in the window.
	*/
	XSetBackground(display,gc,white);
	XSetForeground(display,gc,black);

	/* clear the window and bring it on top of the other windows */
	XClearWindow(display, win);
	XMapRaised(display, win);
};

int main() {
    init_x();
    return 0;
}