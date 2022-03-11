#include <setjmp.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    END_GENERATOR = -1,
    START_GENERATOR = 0
} GeneratorState;

jmp_buf mainTask, childTask;

#define YIELD(thisTask,otherTask, val)      if(!setjmp(thisTask)) longjmp(otherTask, val)
#define GENERATE(val, thisTask, generator)  val = setjmp(thisTask); if (!val) longjmp(generator, 1); if (val == END_GENERATOR) break;
#define START_GENERATOR(task)               if ( !setjmp(task) ) 

struct GeneratorState 
{
    int handle;
    jmp_buf env;
    int data;       // data to return could be a pointer to void
};

int nextNumber(int n) {
    // the buffer is just adding some space on the stack
    // not quite sure why 
    int x = 87;
    // so in memory the stack appears to place the m=n variable after x
    // and then we have our buffer.  When we jump back in it overwrites the final
    // bytes of this buffer for some reason.  But when we next jump back in afterwards
    // it only ever overwrites the last bytes.  I wonder if this is an alignment issue
    
    int buffer[4];    // this is 4 * 4 bytes + 1 for the buffer
    for( int jj=0; jj < 4; jj++ ) {
        buffer[jj] = 53+jj;
    }
    buffer[15] = 43;
    int m = n;
    for(int ii=0; ii < m; ii++ ) {
        printf("(PRE) nextNumber %d %d %d\n",ii, m, n);
        YIELD(childTask, mainTask, ii);
        printf("(POST) nextNumber %d %d %d\n",ii, m, n);
    }
    printf("After generator loop\n");
    longjmp(mainTask, -1);
}


int main() {
    int n = 0;
    int counter  = 0;
    START_GENERATOR(mainTask) { // this marks where we want to return to
        n = nextNumber(97);
    }
    // I think the reason why the memory eats 16 bytes into the buffer in the child function
    // is something to do with the new allocations in main AFTER the setjmp has been created.
    // so the buffer is creating some sort of landing area for where the stack will be restored???
    printf("[BEFORE] n=%d\n", n);
    while(1) {
        GENERATE(n, mainTask, childTask);
        printf("[%d] (AFTER SETJMP(mainTask) n=%d\n",counter, n);
        printf("[IN] n=%d\n", n);
        counter ++;
    }
    return 0;
}