#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

typedef struct _window_list {
    char const * name;
    Display* display;
    int screen;
    Window root;
    Window window;
    XEvent event;
    GC graphicsContext;
    
    int fd;     // XConnectionNumber
    int x;
    int y;
    int width;
    int height;
    int depth;
    int border_width;

    pthread_t thread;
    int state;

    struct _window_list* next;
} WindowList;

pthread_mutex_t windowListMutex = PTHREAD_MUTEX_INITIALIZER;
WindowList* windowListHead = NULL;
WindowList* windowList = NULL;

WindowList* window_default(char * name) {
    WindowList* new_window = (WindowList*) malloc(sizeof(WindowList));
    new_window->name = name;
    new_window->x = 100;
    new_window->y = 100;
    new_window->width = 100;
    new_window->height = 100;
    new_window->border_width = 1;
    new_window->next = NULL;
    new_window->state = 0;
    return new_window;
}

void window_list_add(WindowList* new_window) {
    pthread_mutex_lock(&windowListMutex);
    if ( windowList == NULL ) {
        windowListHead = new_window;
        windowList = new_window;
        pthread_mutex_unlock(&windowListMutex);
        return;
    }
    new_window->next = windowList;
    windowList = new_window;
    pthread_mutex_unlock(&windowListMutex);
}

void window_list_wait() {
    WindowList* runner;
    int still_running = 1;
    while (still_running) {
        runner = windowList;
        still_running = 0;
        while ( runner ) {
            if ( runner->state >= 0 ) {
                still_running = 1;
            }
            runner = runner->next;
        }
        sched_yield();
    }
}

int window_list_length() {
    if ( !windowList) return 0;
    WindowList* runner = windowList;
    int counter = 0;
    while (runner) {
        counter++;
        runner = runner->next;
    }
    return counter;
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

void* x11_event_loop(void* new_window);

void x11_create_window(WindowList* new_window) {
    window_list_add(new_window);

    // open a connection to the  server
    new_window->display = XOpenDisplay(NULL);
    if (new_window->display == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }
    int fd = XConnectionNumber(new_window->display);
    printf("[%s] DISPLAY: %d\n", new_window->name, fd);
    new_window->fd = fd;

    new_window->screen = XDefaultScreen(new_window->display);
    new_window->root = XRootWindow(new_window->display, new_window->screen); // is this the desktop?
    unsigned int blackPixel = BlackPixel(new_window->display, new_window->screen);
    unsigned int whitePixel = WhitePixel(new_window->display, new_window->screen);

    printf("Creating new window at %d,%d\n", new_window->x, new_window->y);
    new_window->window = XCreateSimpleWindow(new_window->display,           // pointer to X server 
                                new_window->root,         // parent to window
                                new_window->x,                 // x
                                new_window->y,                 // y
                                new_window->width,                // width
                                new_window->height,                // height
                                new_window->border_width,                  // border width
                                blackPixel,         // border color
                                whitePixel);        // background

    
    XSelectInput(new_window->display, new_window->window, ExposureMask | KeyPressMask | 
                                    ButtonPressMask |
                                    ButtonReleaseMask | 
                                    ButtonMotionMask );
                                    //| PointerMotionMask);
    
    // https://stackoverflow.com/questions/11069666/cannot-get-xcreatesimplewindow-to-open-window-at-the-right-position
    XSizeHints    my_hints = {0};

    my_hints.flags  = PPosition | PSize;     /* I want to specify position and size */
    my_hints.x      = new_window->x;       /* The origin and size coords I want */
    my_hints.y      = new_window->y;
    my_hints.width  = new_window->width;
    my_hints.height = new_window->height;

    XSetNormalHints(new_window->display, new_window->window, &my_hints);  /* Where new_window is the new window */


    // without this mapping the window is invisible
    XMapWindow(new_window->display, new_window->window);
    new_window->graphicsContext = DefaultGC(new_window->display, new_window->screen);

    XMoveWindow(new_window->display, new_window->window, new_window->x, new_window->y);

    printf("Starting new window thread\n");
    pthread_create(&new_window->thread, NULL, x11_event_loop, new_window);
}

void* x11_event_loop(void* data)
{
    WindowList* new_window = (WindowList*) data;
    printf("In new window thread %s\n", new_window->name);
    new_window->state = 1;
    while (1)
    {
        XNextEvent(new_window->display, &new_window->event);
        printf("%d %s EVENT: %d\n", new_window->fd, new_window->name, new_window->event.type);
        if (new_window->event.type == Expose)
        {
            // DefaultGC(display, screen)
            XFillRectangle(new_window->display, new_window->window, new_window->graphicsContext, 20, 20, 10, 10);
    
            XDrawString(new_window->display, new_window->window, new_window->graphicsContext, 10, 50, new_window->name, strlen(new_window->name));
        }
        if (new_window->event.type == KeyPress) {
            break;
        }
        if (new_window->event.type == ButtonPress) {
            printf("[%s] You pressed a button at (%i,%i)\n", new_window->name, new_window->event.xbutton.x, new_window->event.xbutton.y);
        }
        if (new_window->event.type == ButtonRelease) {
            printf("You released a button at (%i,%i)\n", new_window->event.xbutton.x, new_window->event.xbutton.y);

            int x, y, width, height, borderWidth, depth;
            XWindowAttributes attr = {0};
            XGetWindowAttributes(new_window->display, new_window->window, &attr);
            XGetGeometry(new_window->display, new_window->window, &new_window->root, &x, &y, &width, &height, &borderWidth, &depth);
            printf("%s Geometry: %d,%d %dx%d %d %d\n", new_window->name, x, y, width, height, borderWidth, depth);
            printf("%s Attributes: %d,%d %dx%d %d %d\n", new_window->name, attr.x, attr.y, attr.width, attr.height, attr.border_width, attr.depth);
            if ( new_window->event.xbutton.x > width || new_window->event.xbutton.x < 0 || new_window->event.xbutton.y < 0 || new_window->event.xbutton.y > height) {
                printf("outside window\n");
                printf("Creating second window\n");
                char buffer[100];
                memset(buffer, 0, sizeof(char) * 100);
                sprintf(buffer, "window %d", window_list_length()+1);
                int n = strlen(buffer);
                char* nameBuffer = (char*) malloc(sizeof(char) * (n + 1));
                strcpy(nameBuffer, buffer);
                WindowList* second_window = window_default(nameBuffer);
                // TODO(TM) this should relaly be x,y from Geometry call
                // but for some reason we get the same geometry not matter
                // what we call. 
                second_window->x = attr.x + new_window->event.xbutton.x - new_window->width / 2;
                second_window->y = attr.y + new_window->event.xbutton.y - new_window->height / 2;
                printf("before x11_create_window call\n");
                x11_create_window(second_window);    
            }
        }
        if (new_window->event.type == MotionNotify) {
            printf("You moved the mouse at (%i,%i)\n", new_window->event.xbutton.x, new_window->event.xbutton.y);
        }
        sched_yield();
    }
    new_window->state = -1;
}

int main(void)
{
    WindowList* new_window = window_default("first");
    x11_create_window(new_window);
    printf("do I ever get here first\n");
    window_list_wait();
    printf("do I ever get here\n");
    window_list_close_all();
    return 0;
}