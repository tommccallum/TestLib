// In this file we experiment with using setjmp and longjmp for 
// writing a yield type operation for cooperative multitasking
// Our original mvp is from https://en.wikipedia.org/wiki/Setjmp.h#Cooperative_multitasking
// I have added comments to work out what is happening
#include <setjmp.h>
#include <stdio.h>

// here we have 2 jmp_buf which are storing where both functions have got to
jmp_buf mainTask, childTask;

// void call_with_cushion();
void child();

// we start here in main
int main() {
    // the first time we enter this function we will enter this if statement
    // so before we yield this will be 0 and so we will call call_with_cushion.
    // when we yield this will be one and so we will go on to the rest of the
    // main function
    if (!setjmp(mainTask)) {
        //call_with_cushion(); // child never returns, yield
        child();
    } // execution resumes after this "}" after first time that child yields


    while (1) {                         // infinite loop
        printf("Parent\n");             
        
        if (!setjmp(mainTask))          // the first time this will be 0, so we then yield back to child
            longjmp(childTask, 1);      // yield - note that this is undefined under C99
    }
}

// void call_with_cushion() {
//     // what are we reserving space for?
//     // it does not seem to be required if we comment it out.
//     // char space[1000]; // Reserve enough space for main to run
//     // space[999] = 1; // Do not optimize array out of existence
//     child();
// }

void child() {
    while (1) {
        printf("Child loop begin\n");
        
        if (!setjmp(childTask)) {
            printf("(1) setjmp(childTask) is 0\n");
            longjmp(mainTask, 1); // yield - invalidates childTask in C99
        }

        printf("Child loop end\n");

        if (!setjmp(childTask)) {
            printf("(2) setjmp(childTask) is 0\n");
            longjmp(mainTask, 1); // yield - invalidates childTask in C99
        }
    }

    /* Don't return. Instead we should set a flag to indicate that main()
       should stop yielding to us and then longjmp(mainTask, 1) */
}