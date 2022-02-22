#define USE_EXCEPTION_MAIN
#include "exceptlib.h"
#include "assert.h"

void test_single_try_catch() 
{
  TRY
    div(2,0);
    assert(1==0);             // we assert here as we should not reach it
  CASE DIVIDE_BY_ZERO:
    assert(_EXCEPTION_LIST->type == DIVIDE_BY_ZERO);
  END_TRY
  assert(isExceptionListEmpty());
}

int ApplicationEntry(int argc, char** argv, char** env) {
  (void) argc;
  (void) argv;
  (void) env;
  test_single_try_catch();
  return 0;
}