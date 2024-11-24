#include <inttypes.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifndef NUM_WISHES
#define NUM_WISHES 2
#endif /* NUM_WISHES */

#ifndef UNROLL_WISHES
#define UNROLL_WISHES 1
#endif /* UNROLL_WISHES */


#if NUM_WISHES == 2
#define NUM_WISHES_STR "two"
#elif NUM_WISHES == 3
#define NUM_WISHES_STR "three"
#else
#error "Sorry, no textual variant of NUM_WISHES known :("
#endif

const char egg[] = "https://youtu.be/zxDdML-Rh7I?t=89";

const char* num_ords[] = {
  "first",
  "second",
  "third",
};
_Static_assert(sizeof(num_ords)/sizeof(*num_ords) >= NUM_WISHES, "Not enough ordinals present for NUM_WISHES");


void setup() {
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
}


// Unfortunately, I could not convince GCC to unroll the loop
// with a pragma and -funroll-loops
// Well, let's do it by hand, then...
#define DO_WISH(nr, pos, dat, filp) do { \
    printf("What address do you desire for your %s wish?\n", num_ords[nr]); \
    scanf("%zx", &pos); \
    puts("And what byte do you wish here?"); \
    scanf("%"SCNx8, &dat); \
    \
    puts("✨ As you wish ✨\n"); \
    \
    filp = fopen("/proc/self/mem", "wb"); \
    fseek(filp, pos, SEEK_SET); \
    fwrite(&dat, 1, 1, filp); \
  } while (0)

// https://dev.to/sgf4/repeat-macro-in-c-2hh0
#define REPEAT_1(FN) FN(0)
#define REPEAT_2(FN) REPEAT_1(FN) FN(1)
#define REPEAT_3(FN) REPEAT_2(FN) FN(2)
#define REPEAT_4(FN) REPEAT_3(FN) FN(3)
#define REPEAT_5(FN) REPEAT_4(FN) FN(4)
#define REPEAT_6(FN) REPEAT_5(FN) FN(5)
#define REPEAT_7(FN) REPEAT_6(FN) FN(6)
#define REPEAT_8(FN) REPEAT_7(FN) FN(7)
#define REPEAT_9(FN) REPEAT_8(FN) FN(8)
#define REPEAT_10(FN) REPEAT_9(FN) FN(9)

#define REPEAT(FN, N) REPEAT_##N(FN)
// We need some indirection here to resolve the macro value. Otherwise it concats the macro name
#define RREPEAT(FN, N) REPEAT(FN, N)

int main() {
  FILE* files[NUM_WISHES] = { NULL };

  setup();

  puts("I'm a genie in an ELF and I can grant you " NUM_WISHES_STR " wishes. Choose them wisely!");

  puts("Kind as I am, I will grant you two freebies before we begin:\n");
  char buf[64];
  size_t count;
  FILE* map = fopen("/proc/self/maps", "r");
  while ((count = fread(buf, 1, sizeof(buf), map)) != 0) {
    fwrite(buf, 1, count, stdout);
  }
  fclose(map);

  printf("My card deck stacks %p cards\n", files);


#if UNROLL_WISHES != 0
  size_t pos;
  uint8_t dat;

  #define REPEAT_DO_WISH(i) DO_WISH(i, pos, dat, files[i]);
  RREPEAT(REPEAT_DO_WISH, NUM_WISHES)
  /* The data has been "written", but not written. */
#else
  for (size_t i = 0; i < NUM_WISHES; i++) {
    size_t pos;
    uint8_t dat;

    DO_WISH(i, pos, dat, files[i]);
  }
#endif /* Wish unrolling */


  puts("I have granted you your wishes. Farewell!");

  /* The file get closed and flushed by glibc on exit, like magic ✨ */
  return 0;
}
