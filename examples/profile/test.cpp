#include "foo_ispc.h"

using namespace ispc;

int main() {
  double arr[1000000];

  ISPC_PROFILE_BEGIN(ISPC_PROFILE_ALL_NO_PCM)
  foo(arr, 1000000);
  ISPC_PROFILE_END

  return 0;
}
