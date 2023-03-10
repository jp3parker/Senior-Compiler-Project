/*
  You can add comments like this.
  
  Note that in functions you must declare all variables
  before doing anything else.
*/

int main2(int i) {
  int a;
  a = 0;
  while (a < 10) {
    output(i);
    i = i + 1;
    a = a + 1;
  }
  output(i);
  return 999;
}

int main() {
  output(main2(999));
  return 0;
}
