// Library version to be used with test_exceptions.c
// How to implement exceptions
// Based on this webpage: http://www.on-time.com/ddj0011.htm

// use USE_EXCEPTION_DEBUG  to turn on debugging messages
// use USE_EXCEPTION_MAIN   to use catch-all main and ApplicationEntry

// this gives us access to printf which we will use for output
#include <stdio.h>

// this gives us access to the C function setjmp and longjmp
#include <setjmp.h>

// we need malloc to allocate memory for each exception state
#include <malloc.h>

// for backtrace and backtrace_symbols functionality
#include <execinfo.h>

// string functions such as strlen
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdlib.h>
#include <string.h>

// If we are starting using the given main function then there will be 
// at least 1 try...catch state on our linked list.
int EMPTY_EXCEPTION_LIST_SIZE = 1;

// This is our list of possible exceptions that we can handle.
// TODO(tom) change this to be a struct so we can inherit and create new user exceptions.
typedef enum {
    NO_EXCEPTION = 0,
    DIVIDE_BY_ZERO = -3
} ExceptionType;

// This is our State struct which we use as the nodes in our linked list.
typedef struct ExceptionState_t
{
    jmp_buf                     env;
    ExceptionType               type;
    char const *                fn;
    int                         line;
    char const *                raise_fn;
    int                         raise_line;
    char *                      trace;
    struct ExceptionState_t*    next;
} ExceptionState;

// forward declare our functions that will be used with our TRY...CATCH block
ExceptionState* exceptlib_pushExceptionState(char const * fn, int lineno);
void exceptlib_popExceptionState();
int exceptlib_exceptionListLength();
int exceptlib_isExceptionListEmpty();
void exceptlib_debug(ExceptionType type, char const * fn);
void exceptlib_trace();

// This is a user defined function that will be called by the function 'main'.
int ApplicationEntry(int argc, char** argv, char** env);


// This is a globally accessible linked list.
// We don't want this to be used outside, hence the initial underscore.
ExceptionState* _EXCEPTION_LIST = NULL;


ExceptionState* exceptlib_pushExceptionState(char const * fn, int lineno)
{
    ExceptionState * new_state;
    new_state = (ExceptionState*) malloc(sizeof(ExceptionState));
    new_state->type = NO_EXCEPTION;
    new_state->fn   = fn;
    new_state->line = lineno;
    new_state->raise_fn   = NULL;
    new_state->raise_line = -1;
    new_state->trace = NULL;
    new_state->next = _EXCEPTION_LIST;
    _EXCEPTION_LIST = new_state;
#ifdef USE_EXCEPTION_DEBUG
    printf("pushExceptionState (len: %d)\n", exceptlib_exceptionListLength());
#endif
    return _EXCEPTION_LIST;
}

void exceptlib_popExceptionState() {
    int n2 = exceptlib_exceptionListLength();
    printf("(start) popExceptionState (len: %d)\n", n2);
    ExceptionState* current = _EXCEPTION_LIST;
    _EXCEPTION_LIST = _EXCEPTION_LIST->next;
    if ( current->trace != NULL ) {
        free(current->trace);
    }
    free(current);
#ifdef USE_EXCEPTION_DEBUG
    int n = exceptlib_exceptionListLength();
    printf("popExceptionState (len: %d)\n", n);
#endif
}

int exceptlib_exceptionListLength() {
    ExceptionState* runner = _EXCEPTION_LIST;
    int counter = 0;
    while ( runner != NULL ) {
        runner = runner->next;
        counter++;
    }
    return counter;
}

int exceptlib_isExceptionListEmpty() {
    return exceptlib_exceptionListLength() == EMPTY_EXCEPTION_LIST_SIZE;
}

void exceptlib_debug(ExceptionType type, char const * fn) {
    printf("[DEBUG] Exception of type %d was thrown from function '%s'\n", type, fn);
}

void exceptlib_trace_helper(ExceptionState* next, int index) {
    if ( next == NULL ) {
        return;
    }
    if ( next->trace != NULL ) {
        printf("%s", next->trace);
    }
    if ( next->raise_fn != NULL ) {
        printf("  [%d] %s:%d\n", index, next->raise_fn, next->raise_line);
        index += 1;
    }
    printf("  [%d] %s:%d\n", index, next->fn, next->line);
    exceptlib_trace_helper(next->next, index + 1);
}

void exceptlib_trace(void) {
    printf("This is not a stack trace, but a trace through all the try...catch blocks up to the point of the RAISE.\n");
    printf("Trace:\n");
    ExceptionState* runner = _EXCEPTION_LIST;
    exceptlib_trace_helper(runner, 0);

    printf("Example stack trace, using glibc backtrace, this needs to be added when we Raise an exception:\n");
    void* callstack[128];
    int i, frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    // start at 1 as we don't care about this function in the stack trace
    for (i = 1; i < frames; ++i) {
        printf("  [%d] %s\n", i-1, strs[i]);
    }
    free(strs);
}

void exceptlib_updateExceptionState(char const * fn, int linenum) {
    _EXCEPTION_LIST->raise_fn     = fn;
    _EXCEPTION_LIST->raise_line   = linenum;

    // save the stack trace as a string to the head of the Exception
    void* callstack[128];
    char buffer[255];
    int i, frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    // start at 1 as we don't care about this function in the stack trace
    int totalStringLength = 0;
    for (i = 1; i < frames; ++i) {
        memset(buffer, 0,255);
        sprintf(buffer, "  [%d] %s\n", i-1, strs[i]);
        totalStringLength += strlen(buffer)+1;
    }
    if ( totalStringLength > 0 ) {
        totalStringLength = totalStringLength+ 1;
        char* traceBuffer = (char*) malloc(sizeof(char) * totalStringLength);
        memset(traceBuffer, 0, totalStringLength);
        int b = 0;
        for (i = 1; i < frames; ++i) {
            memset(buffer,0,255);
            snprintf(buffer, 255, "  [%d] %s\n", i-1, strs[i]);
            strncpy(&traceBuffer[b], buffer, strlen(buffer));
            b += strlen(buffer);
        }
        _EXCEPTION_LIST->trace = traceBuffer;
    }
    free(strs);
}

#ifdef USE_EXCEPTION_DEBUG
#   define LOG_DEBUG(x,y) exceptlib_debug(x,y)
#else
#   define LOG_DEBUG(x,y)
#endif

// Here we define the usual syntax for exceptions
// We use a switch statement to get as close as the normal excepted syntax.
//
// these lines must be in the same function otherwise the longjmp fails
#define TRY         _EXCEPTION_LIST->type = setjmp(exceptlib_pushExceptionState(__func__, __LINE__)->env); switch( _EXCEPTION_LIST->type ) { case 0:
#define END_TRY     } exceptlib_popExceptionState();
#define RAISE(x)    LOG_DEBUG(x,__func__); exceptlib_updateExceptionState(__func__, __LINE__); longjmp(_EXCEPTION_LIST->env, x)
#define CASE        break; case
#define DEFAULT     break; default
#define FINALLY     } switch(1) { case 1:


// int subfn(void) {
//     TRY
//         div(2,0);
//         printf("subfn success\n");
//         break;
//     CASE DIVIDE_BY_ZERO:
//         printf("subfn divide by zero\n");
//         break;
//     END_TRY

//     return 0;
// }


// int ApplicationEntry(int argc, char** argv, char** env) {
//     TRY
//         TRY
//             div(2,0);
//             printf("success\n");
//         CASE DIVIDE_BY_ZERO:
//             printf("inner divide by zero\n");
//         END_TRY
//     CASE DIVIDE_BY_ZERO:
//         printf("divide by zero\n");
//     FINALLY
//         printf("always do\n");
//     END_TRY

//     // we raise an error outside of a try loop to test our
//     // catch all
// //    RAISE(DIVIDE_BY_ZERO);
//     return 53;
// }


#ifdef USE_EXCEPTION_MAIN

// We include a specialised main function that will hand off to a user defined ApplicationEntry function.
// This is done so that we can catch any exceptions which have not been wrapped in specific try...catch
// clauses.

int main(int argc, char** argv, char** env) {
    // here we want to catch any exceptions which have not been caught
    // and tell the user about them.
    ExceptionState* runner = NULL;
    ExceptionType lastExceptionCode = NO_EXCEPTION;
    int returnCode = 0;
    TRY
        // if this is an error then returnCode will be 
        // the exception type value
        returnCode = ApplicationEntry(argc, argv, env);
    DEFAULT:
        runner = _EXCEPTION_LIST;
        int depth = exceptlib_exceptionListLength();
        while ( runner != NULL ) {
            if ( lastExceptionCode == 0 ) {
                lastExceptionCode = runner->type;
            }
            if ( runner->type != 0 ) {
                printf("[%d] Unhandled exception %d\n", depth, runner->type);
            }
            runner = runner->next;
            depth--;
        }
        // if ( _EXCEPTION_LIST->next != NULL ) {
        //     int n = exceptionListLength();
        //     printf("Unhandled exception handler found (len: %d)\n", n-1);
        // }
    END_TRY

    int n = exceptlib_exceptionListLength();
    printf("(EXIT) Exception list length: %d)\n", n);
    if ( lastExceptionCode != NO_EXCEPTION ) {
        // Linux only takes the bottom 8 bits so from 0 to 255
        // as the exit code so we need to abs
        return -lastExceptionCode;
    }
    return returnCode;
}

#endif


/**
// jmp_buf is a structure that saves our environment
jmp_buf jumper;

int div(int a, int b) {
    if ( b == 0 ) {
        // loads the execution context saved by the last call to setjmp
        // this function will never return as we are setting the instruction
        // pointer to a complete new point.
        // longjmp(env, status)
        //  where status is the value to return from setjmp
        longjmp(jumper, -3);
    }
    return a / b;
}

void main(void) {
    if ( setjmp(jumper) == 0 ) {
        int z2 = div(7,1);
        printf("z2: %d\n", z2);
        int z = div(7,0);
    } else {
        printf("an error occurred!\n");
    }
}
*/

