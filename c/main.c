// user could change this and then all function
// names would start with this prefix
#define APP_PREFIX          testlib
#define MOCK_PREFIX         mock
#define REAL_PREFIX         real

// Creates a string "x"
#define STR_HELPER(x)       #x
#define STR(x)              STR_HELPER(x)

// This will concatenate 2 macros expanding them if
// called from another macro.
#define CONCAT_HELPER(x ,y)     x ## y
#define CONCAT(x,y)             CONCAT_HELPER(x,y)

#define PACKAGE(x)              CONCAT(APP_PREFIX,CONCAT(_,x))
#define MOCK(x)                 PACKAGE(CONCAT(MOCK_PREFIX,_##x))
#define REAL(x)                 PACKAGE(CONCAT(REAL_PREFIX,_##x))



// here testlib_ is the package name
int REAL(add)(int x, int y) {
    return x + y;
}

int MOCK(add)(int x) {
    (void) x;
    return 5;
}

struct SpyState {
    int counter;
};

int REAL(add)(int x, int y, SpyState** state) {
    state->counter++;
    return REAL(add)(x,y);
}

#define USE_MOCKS 1

#ifdef USE_MOCKS
#   define add MOCK(add)
#else
#   define add PACKAGE(add)
#endif

int main() {
    int z = add(2,3);
    return 0;
}
