#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "instrument.h"

#define NUM_LANES 8

static void mask_to_str(uint64_t mask, char *buffer) {
  for (int i = 0; i < NUM_LANES; i++) {
    buffer[i] = (mask >> (NUM_LANES - 1 - i)) & 0x1 ? '1' : '0';
  }
}

void ISPCInstrument(const char *fn, const char *note,
    int line, uint64_t mask) {
  char buffer[NUM_LANES + 1];
  memset(buffer, 0, (NUM_LANES + 1) * sizeof (char));
  mask_to_str(mask, buffer);
  printf("[%s %d] %s %s\n", fn, line, note, buffer);
}
