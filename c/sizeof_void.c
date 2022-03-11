#include <stdio.h>
int main() {
    printf("void: %lu\n", sizeof(void));
    printf("void * int: %lu\n", sizeof(void) * sizeof(int));
}