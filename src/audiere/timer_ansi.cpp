#include <time.h>
#include "timer.h"


audiere::u64 audiere::GetNow() {
  return audiere::u64(1000000) * clock() / CLOCKS_PER_SEC;
}
