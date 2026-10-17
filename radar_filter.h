#pragma once

// Pure filtering logic (no Arduino dependencies) so it can be unit-tested on a PC.
//
// Takes `count` raw readings (cm) taken at the same angle and returns:
//   - the filtered distance in cm, or
//   - maxRange when no object is confirmed.
//
// A reading counts as "object" if 1 <= value < maxRange (maxRange itself means
// "nothing detected"). Rules:
//   1. A majority of the readings must see an object (rejects single spikes).
//   2. Readings further than `tolerance` cm from the median are discarded.
//   3. The remaining readings must still be a majority; they are averaged.
inline int filterDistanceSamples(const int *samples, int count, int maxRange, int tolerance) {
  int valid[8];
  int n = 0;
  for (int i = 0; i < count && n < 8; i++) {
    if (samples[i] >= 1 && samples[i] < maxRange) valid[n++] = samples[i];
  }
  if (n * 2 <= count) return maxRange;

  // insertion sort (n is tiny)
  for (int i = 1; i < n; i++) {
    int key = valid[i];
    int j = i - 1;
    while (j >= 0 && valid[j] > key) {
      valid[j + 1] = valid[j];
      j--;
    }
    valid[j + 1] = key;
  }

  int median = valid[n / 2];
  long sum = 0;
  int kept = 0;
  for (int i = 0; i < n; i++) {
    int d = valid[i] - median;
    if (d < 0) d = -d;
    if (d <= tolerance) {
      sum += valid[i];
      kept++;
    }
  }
  if (kept * 2 <= count) return maxRange;

  return (int)((sum + kept / 2) / kept);  // rounded average
}
