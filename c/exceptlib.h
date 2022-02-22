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
    struct ExceptionState_t*    next;
} ExceptionState;

// forward declare our functions that will be used with our TRY...CATCH block
ExceptionState* exceptlib_pushExceptionState();
void exceptlib_popExceptionState();
int exceptlib_exceptionListLength();
int exceptlib_isExceptionListEmpty();
void exceptLib_debug(ExceptionType type, char const * fn);

// This is a user defined function that will be called by the function 'main'.
int ApplicationEntry(int argc, char** argv, char** env);


// This is a globally accessible linked list.
// We don't want this to be used outside, hence the initial underscore.
ExceptionState* _EXCEPTION_LIST = NULL;


ExceptionState* exceptlib_pushExceptionState()
{
    ExceptionState * new_state;
    new_state = (ExceptionState*) malloc(sizeof(ExceptionState));
    new_state->type = NO_EXCEPTION;
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

void exceptLib_debug(ExceptionType type, char const * fn) {
    printf("[DEBUG] Exception of type %d was thrown from function '%s'\n", type, fn);
}

#ifdef USE_EXCEPTION_DEBUG
#   define LOG_DEBUG(x,y) exceptLib_debug(x,y)
#else
#   define LOG_DEBUG(x,y)
#endif

// Here we define the usual syntax for exceptions
// We use a switch statement to get as close as the normal excepted syntax.
//
// these lines must be in the same function otherwise the longjmp fails
#define TRY         _EXCEPTION_LIST->type = setjmp(exceptlib_pushExceptionState()->env); switch( _EXCEPTION_LIST->type ) { case 0:
#define END_TRY     } exceptlib_popExceptionState();
#define RAISE(x)    LOG_DEBUG(x,__func__); longjmp(_EXCEPTION_LIST->env, x)
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

