int foo(int a, int b);

void main(int f) {
  if (1) {
    while (2)
      return 3;
  }
  else {
    return 4;
  }
}
