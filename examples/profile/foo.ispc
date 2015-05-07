#define MIN(a, b) ((a) < (b) ? (a) : (b))

task void task1() {
  foreach (i = 1 ... 1000) {
    int y = 0;
    for (int x = 0; x < i * 2; x++) {
      y++;
    }
  }
}

task void task2() {
  uniform int y = 0;
  for (int x = 0; x < 5; x++) {
    y++;
  }
}

void bar(int a) {
  int b = a + 100;
  return;
}

export void foo(uniform double arr[], uniform int len) {
  launch task1();
  launch task2();
  int a = programIndex;
  int b = 1;
  int c = 1;
  int d = 1;

  // Testing nested if statements and else if statements.
  if (a == 0) {
    b = 0;
  } 
  else {
    if (a == 1) {
      b = 1;
      a = 2;
      d = 2;
    }
    else if (a == 2) {
      b = 2;
      a = 2;
      d = 2;
    }
    else {
      b = 3;
      a = 2;
      d = 2;
    }
  }

  // Test coherent control flow detection.
  if (c == 1) {
    c = 0;
  } else {
    c = -1;
  }

  // Testing function call.
  bar(b);

  // Test unmask.
  if (a == 0) {
    unmasked {
      for (int i = 0; i < MIN(8, len); i++) {
        b += (int) arr[i];
      }
    }
  }

  // Test switch
  switch (b) {
    case 0:
      b = 1;
      break;
    case 1:
      b = 2;
    default:
      b = 0;
      break;
  }
  
  // Test foreach.
  foreach (i = 1 ... 5) {
    b = 0;
  }
}
