#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "intel_pcm/cpucounters.h"

extern "C" {
  void ISPCProfileInit(const char *fn);
  void ISPCProfileComplete();
  void ISPCProfileStart(const char *note, int line, int type, int task,
      uint64_t mask);
  void ISPCProfileEnd();
  void ISPCProfileIteration(const char *note, int line, int64_t mask);
  void ISPCProfileIf(const char *note, int line, int64_t mask);
}

// TODO remove this assumption
static int num_lanes = 8;

// Intel Performance Counter Monitor
static PCM *monitor;
static SystemCounterState before_sstate;

static void mask_to_str(uint64_t mask, char *buffer) {
  for (int i = 0; i < num_lanes; i++) {
    buffer[i] = (mask >> (num_lanes - 1 - i)) & 0x1 ? '1' : '0';
  }
}

void ISPCProfileInit(const char *fn, int total_lanes, int verbose) {
  // TODO implement
  (void) fn;
  (void) verbose;
  num_lanes = total_lanes;

  monitor = PCM::getInstance();
  monitor->program(PCM::DEFAULT_EVENTS, NULL);

  if (monitor->program() != PCM::Success) {
    // TODO report pcm failed
    return;
  }
}

void ISPCProfileComplete() {
  // TODO implement
}

void ISPCProfileStart(const char *note, int line, int type, int task, 
    uint64_t mask) {
  (void) task;
  (void) type;
  char buffer[num_lanes + 1];
  memset(buffer, '\0', (num_lanes + 1) * sizeof (char));
  mask_to_str(mask, buffer);
  printf("[%d] %s %s\n", line, note, buffer);

  // Get Intel performance monitor state
  before_sstate = getSystemCounterState();
}

void ISPCProfileEnd() {
  SystemCounterState after_sstate = getSystemCounterState();

  double ipc = getIPC(before_sstate, after_sstate);
  double l3_hit = getL3CacheHitRatio(before_sstate, after_sstate);
  uint64_t bytes = getBytesReadFromMC(before_sstate, after_sstate);

  char buffer[512];
  memset(buffer, 0, sizeof (char) * 512);
  sprintf(buffer, "IPC: %f, L3 Cache Hit Ratio: %f, Bytes Read: %ld\n", 
      ipc, l3_hit, bytes);
  printf("%s", buffer);
}

void ISPCProfileIteration(const char *note, int line, int64_t mask) {
  char buffer[num_lanes + 1];
  memset(buffer, '\0', (num_lanes + 1) * sizeof (char));
  mask_to_str(mask, buffer);
  printf("\t[%d] %s %s\n", line, note, buffer);
}

void ISPCProfileIf(const char *note, int line, int64_t mask) {
  char buffer[num_lanes + 1];
  memset(buffer, '\0', (num_lanes + 1) * sizeof (char));
  mask_to_str(mask, buffer);
  printf("\t[%d] %s %s\n", line, note, buffer);
}
