#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern "C" {
  void ISPCProfileInit(const char *fn);
  void ISPCProfileComplete();
  void ISPCProfileStart(const char *note, int line, int type, int task,
      uint64_t mask);
  void ISPCProfileEnd();
}

static int num_lanes;

void mask_to_str(uint64_t mask, char *buffer) {
  for (int i = 0; i < num_lanes; i++) {
    buffer[i] = (mask >> (num_lanes - 1 - i)) & 0x1 ? '1' : '0';
  }
}

void ISPCProfileInit(const char *fn, int total_lanes, int verbose) {
  // TODO implement
  (void) fn;
  (void) verbose;
  num_lanes = total_lanes;
}

void ISPCProfileComplete() {
  // TODO implement
}

void ISPCProfileStart(const char *note, int line, int type, int task, 
    uint64_t mask) {
  (void) task;
  (void) type;
  char buffer[num_lanes + 1];
  memset(buffer, 0, (num_lanes + 1) * sizeof (char));
  mask_to_str(mask, buffer);
  printf("[%d] %s %s\n", line, note, buffer);
}

void ISPCProfileEnd() {
  // TODO implement
}
