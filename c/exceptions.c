// How to implement exceptions
// Based on this webpage: http://www.on-time.com/ddj0011.htm

#include <stdio.h>
// this gives us access to the C function setjmp and longjmp
#include <setjmp.h>
#include <malloc.h>

typedef enum {
    NO_EXCEPTION = 0,
    DIVIDE_BY_ZERO = -3
} ExceptionType;

typedef struct ExceptionState_t
{
    jmp_buf                     env;
    ExceptionType               type;
    struct ExceptionState_t*    next;
} ExceptionState;

ExceptionState* _EXCEPTION_LIST = NULL;

ExceptionState* _pushExceptionState(ExceptionState* head) {
    ExceptionState * new_state;
    new_state = (ExceptionState*) malloc(sizeof(ExceptionState));
    new_state->type = NO_EXCEPTION;
    new_state->next = head;
    head = new_state;
    return head;
}

ExceptionState* pushExceptionState()
{
    _EXCEPTION_LIST = _pushExceptionState(_EXCEPTION_LIST);
    return _EXCEPTION_LIST;
}

int exceptionListLength();

void popExceptionState() {
    ExceptionState* current = _EXCEPTION_LIST;
    _EXCEPTION_LIST = _EXCEPTION_LIST->next;
    free(current);
    int n = exceptionListLength();
    printf("popExceptionState:: Exception list length: %d\n", n);
}

int exceptionListLength() {
    ExceptionState* runner = _EXCEPTION_LIST;
    int counter = 0;
    while ( runner != NULL ) {
        runner = runner->next;
        counter++;
    }
    return counter;
}

void ExceptLib_debug(ExceptionType type, char const * fn) {
    printf("[DEBUG] Exception of type %d was thrown from function '%s'\n", type, fn);
}

// Here we define the usual syntax for exceptions
// We use a switch statement to get as close as the normal excepted syntax.
//
// these lines must be in the same function otherwise the longjmp fails
#define TRY         _EXCEPTION_LIST->type = setjmp(pushExceptionState()->env); switch( _EXCEPTION_LIST->type ) { case 0:
#define END_TRY     popExceptionState(); }
#define RAISE(x)    ExceptLib_debug(x,__func__); longjmp(_EXCEPTION_LIST->env, x)
#define CASE        break; case
#define DEFAULT     break; default
#define FINALLY     popExceptionState(); } switch(1) { case 1:

int div(int a, int b) {
    if ( b == 0 ) {
        RAISE(DIVIDE_BY_ZERO);
    } else {
        return a / b;
    }
}

int subfn(void) {
    TRY
        div(2,0);
        printf("subfn success\n");
        break;
    CASE DIVIDE_BY_ZERO:
        printf("subfn divide by zero\n");
        break;
    END_TRY

    return 0;
}


int ApplicationEntry(int argc, char** argv, char** env) {
    TRY
        TRY
            div(2,0);
            printf("success\n");
        CASE DIVIDE_BY_ZERO:
            printf("inner divide by zero\n");
        END_TRY
    CASE DIVIDE_BY_ZERO:
        printf("divide by zero\n");
    FINALLY
        printf("always do\n");
    END_TRY

    // we raise an error outside of a try loop to test our
    // catch all
//    RAISE(DIVIDE_BY_ZERO);
    return 53;
}

#define USE_EXCEPTION_MAIN

#ifdef USE_EXCEPTION_MAIN
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
        printf("outermost default exception handler\n");
        runner = _EXCEPTION_LIST;
        while ( runner != NULL ) {
            if ( lastExceptionCode == 0 ) {
                lastExceptionCode = runner->type;
            }
            if ( runner->type != 0 ) {
                printf("Unhandled exception %d\n", runner->type);
            }
            runner = runner->next;
        }
        if ( _EXCEPTION_LIST->next != NULL ) {
            int n = exceptionListLength();
            printf("Unhandled exception handler found (len: %d)\n", n-1);
        }
    END_TRY

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

