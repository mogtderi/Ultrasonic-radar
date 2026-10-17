// Host-side test for the distance filter:  g++ -std=c++11 -I../firmware/radar_esp32 test_filter.cpp -o test_filter && ./test_filter
#include <cstdio>
#include "radar_filter.h"

static int failures = 0;

static void check(const char *name, int expected, int a, int b, int c) {
  int samples[3] = {a, b, c};
  int got = filterDistanceSamples(samples, 3, 15, 3);
  if (got != expected) {
    std::printf("FAIL %-34s expected %2d got %2d\n", name, expected, got);
    failures++;
  } else {
    std::printf("ok   %-34s -> %d\n", name, got);
  }
}

int main() {
  check("empty space (all max range)",        15, 15, 15, 15);
  check("steady object at 8 cm",               8,  8,  8,  8);
  check("single spike is rejected",           15,  8, 15, 15);
  check("two close readings are averaged",     9,  8,  9, 15);
  check("outlier dropped, rest averaged",      6,  5,  6, 14);
  check("two far-apart readings rejected",    15,  4, 12, 15);
  check("zero/invalid readings ignored",      15,  0,  0, 15);
  check("noisy object around 10 cm",          10,  9, 10, 11);
  std::printf(failures ? "\n%d test(s) FAILED\n" : "\nall tests passed\n", failures);
  return failures ? 1 : 0;
}
