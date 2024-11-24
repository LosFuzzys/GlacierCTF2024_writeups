/*
 * If you want reproducible C builds don't use _TIME_, _FILE_ or LTO, they
 * require special treatment. This is optional.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  srand(time(NULL));
  printf("Hello world, visitor number %d\n", rand());

  FILE *f = fopen("/flag.txt", "r");
  if(f == NULL) {
    fprintf(stderr, "[!] Error reading the flag\n");
    return 1;
  }

  char flag[256];
  char *r = fgets(flag, 256, f);
  printf("%s\n", flag);

  return 0;
}
