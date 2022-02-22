#define USE_EXCEPTION_DEBUG
#define USE_EXCEPTION_MAIN

#include "exceptlib.h"
#include "assert.h"

int div(int a, int b) {
  printf("Divide %d by %d\n", a, b);
  if ( b == 0 ) {
      RAISE(DIVIDE_BY_ZERO);
  } else {
      return a / b;
  }
}


void test_embedded_try_catch() 
{
  TRY
    TRY
      div(2,0);
      assert(1==0);             // we assert here as we should not reach it
    CASE DIVIDE_BY_ZERO:
      exceptlib_trace();
      assert(_EXCEPTION_LIST->type == DIVIDE_BY_ZERO);
    END_TRY
  CASE DIVIDE_BY_ZERO:
    assert(_EXCEPTION_LIST->type == DIVIDE_BY_ZERO);
  END_TRY
  assert(exceptlib_isExceptionListEmpty());
}


void test_single_try_catch() 
{
  TRY
    div(2,0);
    assert(1==0);             // we assert here as we should not reach it
  CASE DIVIDE_BY_ZERO:
    assert(_EXCEPTION_LIST->type == DIVIDE_BY_ZERO);
  END_TRY
  assert(exceptlib_isExceptionListEmpty());
}

int ApplicationEntry(int argc, char** argv, char** env) {
  (void) argc;
  (void) argv;
  (void) env;
  printf("test_single_try_catch\n");
  test_single_try_catch();
  printf("test_embedded_try_catch\n");
  test_embedded_try_catch();
  return 0;
}