#include <stdio.h>
#include <assert.h>

int main() {
  printf("Start test\n");
  FILE *fp = fopen("/share/texts/num", "r+");
  assert(fp);
  printf("PASS test 1\n");

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  assert(size == 5000);
  printf("PASS test 2\n");


  fseek(fp, 500 * 5, SEEK_SET);
  int i, n;
  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }
  printf("PASS test 3\n");

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }

  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }
  printf("PASS test 4\n");

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1 + 1000);
  }
  printf("PASS test 5\n");

  fclose(fp);

  printf("PASS ALL!!!\n");

  return 0;
}
