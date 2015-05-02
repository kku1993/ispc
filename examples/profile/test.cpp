#include "foo_ispc.h"

using namespace ispc;

int main() {
  double arr[1000000];

  ISPC_PROFILE_BEGIN(1)
  foo(arr, 1000000);
  ISPC_PROFILE_END

  return 0;
}
