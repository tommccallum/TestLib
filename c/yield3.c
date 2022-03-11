// Yield functionality
// What we need to do is allocate a stack for each function
//  that is running.
#include <setjmp.h>
#include <malloc.h>
#include <stdio.h>

#define DEFAULT_STACK_SIZE 1024


typedef enum _TaskManagerSignal {
    TASK_MANAGER_INITIALISE=0,      // when the task manager first loads it gets this signal
    TASK_MANAGER_RUN_NEXT,          // when a task is deleted this signal is sent
    TASK_MANAGER_EXIT_CURRENT_TASK, 
    TASK_MANAGER_EXIT               // when we want the scheduler to end early this signal is sent
} TaskManagerSignal;

typedef enum _TaskState {
    TASK_CREATED,           // when a task is first created, it gets put into this state
    TASK_WAITING,           // when a task has run at least once it gets put into this state
    TASK_RUNNING,           // when a task is currently running it is in this state
    TASK_STOPPED            // when a task has completed but not yet removed its in this state
} TaskState;

struct _TaskManager;

typedef struct _Task 
{
    struct _TaskManager* task_manager;
    
    int index;
    void (*callable)(struct _Task*,void* args);
    void* args;
    jmp_buf env;

    void* stack_base_ptr;
    int stack_size;
    int stack_offset;
    TaskState status;
    
    void* return_values;                // a buffer just of bytes
    int* return_values_size;            // an array of size of return structure in the buffer
    int* return_values_ptr;             // an array of indexes into the buffer

    // linked list pointers to next and previous nodes
    struct _Task* next;
    struct _Task* prev;
} Task;


// our task manager is going to give each process a piece of time
// in this version it will be up to the problem to yield but in the
// next we could add a timer.
typedef struct _TaskManager
{
    // the task manager / scheduler has its own jump buffer so we can move back to where our main program was.
    jmp_buf buf;
    Task* tasklist;
    int current_task;
    int task_list_size;
} TaskManager;


void tasklist_add(TaskManager* manager, Task* task) {
    task->index = manager->task_list_size;
    manager->task_list_size += 1;
    if ( manager->tasklist == NULL ) {
        manager->tasklist = task;
    } else {
        Task* old_head = manager->tasklist;
        task->next = old_head;
        manager->tasklist = task;
    } 
}

Task* tasklist_get(Task* tasklist, int index) {
    Task* runner = tasklist;
    while ( runner != NULL ) {
        if ( runner->index == index ) {
            return runner;
        }
        runner = runner->next;
    }
    return NULL;
}

Task* tasklist_remove(Task** tasklist, int index) {
    Task * runner = *tasklist;
    Task * prev = NULL;
    while ( runner != NULL ) {
        if ( runner->index == index ) {
            if ( prev == NULL ) {
                tasklist = &runner->next;
                runner->next = NULL;
                return runner;
            } else {
                prev->next = runner->next;
                runner->next = NULL;
                return runner;
            }
        }
        prev = runner;
        runner = runner->next;
    }
    return NULL;
}

TaskManager* task_manager_create(void) {
    TaskManager* task_manager = (TaskManager*) malloc(sizeof(TaskManager));
    task_manager->tasklist = NULL;
    task_manager->current_task = -1;
    task_manager->task_list_size = 0;
    return task_manager;
}

Task* task_manager_get_current_task(TaskManager* task_manager) {
    return tasklist_get(task_manager->tasklist, task_manager->current_task);
}

void task_manager_add(TaskManager* task_manager, Task* task) {
    task->task_manager = task_manager;
    tasklist_add(task_manager, task);
    task_manager->task_list_size++;
}

// If a task has not been run before then we use this function
void task_manager_run(TaskManager* task_manager) 
{
    if ( task_manager->task_list_size == 0 ) {
        return;
    }

    int signal = setjmp(task_manager->buf);
    switch(signal) {
    case TASK_MANAGER_EXIT_CURRENT_TASK:
        Task* task = task_manager_get_current_task(task_manager);
        task_destroy(task);
    case TASK_MANAGER_INITIALISE:
    case TASK_MANAGER_RUN_NEXT:
        task_manager_next(task_manager);
    default:
        fprintf(stderr, "Task manager received an invalid signal (%d)", signal);
    }
    task_manager->current_task++;
    Task* task = task_manager_get_current_task(task_manager);
    task_run(task);
}


void task_manager_increment(TaskManager* task_manager) {
    task_manager->current_task = (task_manager->current_task + 1) % task_manager->task_list_size;
}

// If we want to move on to the next task then use this function
void task_manager_next(TaskManager* task_manager) {
    task_manager_increment(task_manager);
    while ( 1 ) {
        Task* current_task = task_manager_get_current_task(task_manager);
        if ( current_task == NULL ) {
            // we have finished all the tasks waiting
            break; 
        }
        if ( current_task->status == TASK_STARTED) {
            task_manager_run(task_manager);
        } else if ( current_task == TASK_WAITING ) {
            current_task->status = TASK_RUNNING;  
            longjmp(current_task->env, 1);
        } else if ( current_task == TASK_RUNNING ) {
            current_task->status = TASK_WAITING;
            task_manager_increment(task_manager);
        }
    }
}

// void scheduler_relinquish(void)
// {
// 	if (setjmp(priv.current->buf)) {
// 		return;
// 	} else {
// 		longjmp(priv.buf, SCHEDULE);
// 	}
// }
void yield(Task* task) {
    TaskManager* task_manager;
    if ( task->task_manager->task_list_size == 0 ) {
        return;
    }
    Task* yielding_task = task_manager_get_current_task(task_manager);
    yielding_task->status = TASK_WAITING;
    task_manager_next(task_manager);

    if (setjmp(yielding_task->env) ) { 
        return;
    } else {
        // jump back to the task manager to move to the next task
        longjmp(task->task_manager->buf, CONTINUE);
    }
}

void task_set_return_int(Task* task, int index, int value) {
    *((int*)&(task->return_values[index])) = value;
}



void task_stop(Task* task) {
    task->status = TASK_STOPPED;
    task_manager_next(task->task_manager);
}

void test_runnable(Task* task, void* args) {
    for( int ii=0; ii < 1000000; ii++ ) {
        printf("[%d] %d", task->index, ii);
        task_set_return_int(task, 0, ii);
        yield(task);
    }
    task_stop(task)
}

Task* task_create(void (*callback)(Task*,void*), void* args) {
    Task* t = (Task*) malloc( sizeof(Task) );
    t->stack_size = DEFAULT_STACK_SIZE;
    void* new_stack = (void*) malloc( sizeof(void) * t->stack_size );
    t->stack = new_stack;
    t->stack_offset = 0;
    t->status = TASK_STARTED;
    t->callable = callback;
    t->args = args;
    t->next = NULL;
    t->prev = NULL;
    return t;
}

// static void scheduler_free_current_task(void)
// {
// 	struct task *task = priv.current;
// 	priv.current = NULL;
// 	free(task->stack_bottom);
// 	free(task);
// }
void task_destroy(Task* task) {
    free(task->stack);
    free(task);
}


void task_run(Task* task) {
    // register means that the value *top must be placed in a CPU register
    register void *top = &(task->stack[task->stack_offset]);
    // +r load a register with the value pointed at by top
    // replace [rs] with the register name
    // volatile to ensure the compiler does not fiddle with the placement of this
    asm volatile(
        "mov %[rs], %%rsp \n"
        : [rs] "+r" (top) ::
    );

    task->status = TASK_RUNNING;
    task->callable(task, task->args);
    // when we get here the stack pointer will be still in the task stack not
    // our real stack so we need to exit cleanly
    task_exit(task);
}

void task_exit(Task* task) {
    TaskManager * task_manager = task->task_manager;
    task_manager_remove_task(task);
    longjmp(task_manager->buf, EXIT_TASK);
}


int main() {
    TaskManager* task_manager = task_manager_create();
 
    void* args = (void*) malloc(sizeof(void) * sizeof(int) * 1);
    *((int*)args) = 7;
    Task* new_task = task_create(&test_runnable, args);
    task_manager_add(task_manager, new_task);
 
    args = (void*) malloc(sizeof(void) * sizeof(int) * 1);
    *((int*)args) = 8;
    new_task = task_create(&test_runnable, args);
    task_manager_add(task_manager, new_task);
 
    task_manager_run(task_manager);
}