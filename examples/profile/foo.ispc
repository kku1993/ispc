#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*
task void foo_task() {
  foreach (i = 1 ... 1000) {
    int y = 0;
    for (int x = 0; x < i * 2; x++) {
      y++;
    }
  }
}
*/

void bar(int a) {
  int b = a + 100;
  return;
}

export void foo(uniform double arr[], uniform int len) {
  //launch foo_task();
  int a = programIndex;
  int b = 1;
  if (a == 0) {
    b = 0;
  } else {
    if (a == 1)
      b = 1;
    else if (a == 2)
      b = 2;
    else
      b = 3;
  }

  bar(b);

  if (a == 0) {
    for (int i = 0; i < MIN(5, len); i++) {
      b += (int) arr[i];
    }
  }
  
  foreach (i = 1 ... 5) {
    b = 0;
  }
}
