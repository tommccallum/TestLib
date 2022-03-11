#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _window_list {
    char const * name;
    Display* display;
    int screen;
    Window root;
    Window window;
    XEvent event;
    GC graphicsContext;
    struct _window_list* next;
} WindowList;

WindowList* windowList = NULL;

void window_list_add(WindowList* new_window) {
    if ( windowList == NULL ) {
        windowList = new_window;
        return;
    }
    new_window->next = windowList;
    windowList = new_window;
}

void window_list_close_all() {
    WindowList* runner = windowList;
    while ( runner ) {
        XCloseDisplay(windowList->display);
        WindowList* old = runner;
        runner = runner->next;
        free(old);
    }
    windowList = NULL;
}
void x11_create_window(char const * name) {
    WindowList* new_window = (WindowList*) malloc(sizeof(WindowList));
    new_window->name = name;
    window_list_add(new_window);

    // open a connection to the  server
    new_window->display = XOpenDisplay(NULL);
    if (new_window->display == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    new_window->screen = XDefaultScreen(new_window->display);
    new_window->root = XRootWindow(new_window->display, new_window->screen); // is this the desktop?
    unsigned int blackPixel = BlackPixel(new_window->display, new_window->screen);
    unsigned int whitePixel = WhitePixel(new_window->display, new_window->screen);
    new_window->window = XCreateSimpleWindow(new_window->display,           // pointer to X server 
                                new_window->root,         // parent to window
                                10,                 // x
                                10,                 // y
                                100,                // width
                                100,                // height
                                1,                  // border width
                                blackPixel,         // border color
                                whitePixel);        // background

    
    XSelectInput(new_window->display, new_window->window, ExposureMask | KeyPressMask | 
                                    ButtonPressMask |
                                    ButtonReleaseMask | 
                                    ButtonMotionMask );
                                    //| PointerMotionMask);
    XMapWindow(new_window->display, new_window->window);
    new_window->graphicsContext = DefaultGC(new_window->display, new_window->screen);
    while (1)
    {
        XNextEvent(new_window->display, &new_window->event);
        printf("EVENT: %d\n", new_window->event.type);
        if (new_window->event.type == Expose)
        {
            // DefaultGC(display, screen)
            XFillRectangle(new_window->display, new_window->window, new_window->graphicsContext, 20, 20, 10, 10);
    
            XDrawString(new_window->display, new_window->window, new_window->graphicsContext, 10, 50, new_window->name, strlen(msg));
        }
        if (new_window->event.type == KeyPress)
            break;
        if (new_window->event.type == ButtonPress) {
            printf("You pressed a button at (%i,%i)\n", new_window->event.xbutton.x, new_window->event.xbutton.y);
        }
        if (new_window->event.type == ButtonRelease) {
            printf("You released a button at (%i,%i)\n", new_window->event.xbutton.x, new_window->event.xbutton.y);

            int x, y, width, height, borderWidth, depth;
            XGetGeometry(new_window->display, new_window->window, &new_window->root, &x, &y, &width, &height, &borderWidth, &depth);
            printf("Geometry: %d,%d %dx%d %d %d\n", x, y, width, height, borderWidth, depth);
            if ( new_window->event.xbutton.x > width || new_window->event.xbutton.x < 0 || new_window->event.xbutton.y < 0 || new_window->event.xbutton.y > height) {
                printf("outside window\n");
                printf("Creating second window\n");
                x11_create_window("window");    
            }
        }
        if (new_window->event.type == MotionNotify) {
            printf("You moved the mouse at (%i,%i)\n", new_window->event.xbutton.x, new_window->event.xbutton.y);
        }
    }

}

int main(void)
{
    Display *display; // The X server
    Window window;    // The application window
    Window window2;
    Window rootWindow;
    XEvent e; // an event object
    const char *msg = "Hello, World!";
    int screen;
    unsigned long blackPixel;
    unsigned long whitePixel;
    GC graphicsContext;

    // open a connection to the  server
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    screen = XDefaultScreen(display);
    rootWindow = XRootWindow(display, screen); // is this the desktop?
    blackPixel = BlackPixel(display, screen);
    whitePixel = WhitePixel(display, screen);
    window = XCreateSimpleWindow(display,           // pointer to X server 
                                rootWindow,         // parent to window
                                10,                 // x
                                10,                 // y
                                100,                // width
                                100,                // height
                                1,                  // border width
                                blackPixel,         // border color
                                whitePixel);        // background

    
    XSelectInput(display, window, ExposureMask | KeyPressMask | 
                                    ButtonPressMask |
                                    ButtonReleaseMask | 
                                    ButtonMotionMask );
                                    //| PointerMotionMask);
    XMapWindow(display, window);
    graphicsContext = DefaultGC(display, screen);
    int secondWindowCreated = 0;
    while (1)
    {
        XNextEvent(display, &e);
        printf("EVENT: %d\n", e.type);
        if (e.type == Expose)
        {
            // DefaultGC(display, screen)
            XFillRectangle(display, window, graphicsContext, 20, 20, 10, 10);
    
            XDrawString(display, window, graphicsContext, 10, 50, msg, strlen(msg));
        }
        if (e.type == KeyPress)
            break;
        if (e.type == ButtonPress) {
            printf("You pressed a button at (%i,%i)\n", e.xbutton.x, e.xbutton.y);
        }
        if (e.type == ButtonRelease) {
            printf("You released a button at (%i,%i)\n", e.xbutton.x, e.xbutton.y);

            int x, y, width, height, borderWidth, depth;
            XGetGeometry(display, window, &rootWindow, &x, &y, &width, &height, &borderWidth, &depth);
            printf("Geometry: %d,%d %dx%d %d %d\n", x, y, width, height, borderWidth, depth);
            if ( e.xbutton.x > width || e.xbutton.x < 0 || e.xbutton.y < 0 || e.xbutton.y > height) {
                printf("outside window\n");
                if ( !secondWindowCreated ) {
                    printf("Creating second window\n");
                    window2 = XCreateSimpleWindow(display,           // pointer to X server 
                                rootWindow,         // parent to window
                                e.xbutton.x,                 // x
                                e.xbutton.y,                 // y
                                100,                // width
                                100,                // height
                                1,                  // border width
                                blackPixel,         // border color
                                whitePixel);        // background
                    secondWindowCreated = 1;
                }
            }
        }
        if (e.type == MotionNotify) {
            printf("You moved the mouse at (%i,%i)\n", e.xbutton.x, e.xbutton.y);
        }
    }

    // Close connection to X server
    window_list_close_all();
    return 0;
}