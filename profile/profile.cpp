#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "intel_pcm/cpucounters.h"
#include "profile_ctx.h"

extern "C" {
  void ISPCProfileInit(const char *fn, int line, int total_lanes, int verbose);
  void ISPCProfileComplete();
  void ISPCProfileStart(const char *note, int start_line, int end_line, 
      int task, uint64_t mask);
  void ISPCProfileEnd();
  void ISPCProfileIteration(const char *note, int line, int64_t mask);
  void ISPCProfileIf(const char *note, int line, int64_t mask);
}

// Indicate whether a profiler is running already.
// Avoid initializing the profiler multiple times.
static bool profile_running = false;

// Vector width that the program is compiled for.
static int num_lanes;

// Intel Performance Counter Monitor
static PCM *monitor;

static ProfileContext *ctx = new ProfileContext();

static void mask_to_str(uint64_t mask, char *buffer) {
  for (int i = 0; i < num_lanes; i++) {
    buffer[i] = (mask >> (num_lanes - 1 - i)) & 0x1 ? '1' : '0';
  }
}

void ISPCProfileInit(const char *file, int line, int total_lanes, int verbose) {
  if (profile_running)
    return;

  profile_running = true;

  printf("Profile init: %s[%d] with %d lanes\n", file, line, total_lanes);

  (void) verbose;
  num_lanes = total_lanes;

  monitor = PCM::getInstance();
  monitor->program(PCM::DEFAULT_EVENTS, NULL);

  PCM::ErrorCode err = monitor->program();
  if (err != PCM::Success) {
    // TODO report pcm failed
    printf("PCM init failed [error = %d].\n", err);
    return;
  }

  printf("Profile Init\n");
}

void ISPCProfileComplete() {
  monitor->cleanup();
  profile_running = false;
  printf("Profile Complete\n");
}

void ISPCProfileStart(const char *note, int start_line, int end_line, int task,
    uint64_t mask) {
  char buffer[num_lanes + 1];
  memset(buffer, '\0', (num_lanes + 1) * sizeof (char));
  mask_to_str(mask, buffer);
  printf("[%d-%d] %s %s\n", start_line, end_line, note, buffer);

  // Get Intel performance monitor state
  SystemCounterState state = getSystemCounterState();

  ProfileRegion *region = new ProfileRegion(note, start_line, end_line, task, 
      mask, state);
  ctx->pushRegion(region);
}

void ISPCProfileEnd() {
  SystemCounterState after_sstate = getSystemCounterState();

  // Remove the profile region from the profiling context.
  ProfileRegion *region = ctx->popRegion();
  if (region == NULL) {
    // TODO stop program 
    printf("ERROR: No region currently in scope can be removed.\n");
  }
  region->updateExitStatus(after_sstate);

  double ipc = region->getRegionIPC();
  double l3_hit = region->getRegionL3HitRatio();
  double l2_hit = region->getRegionL2HitRatio();
  uint64_t bytes = region->getRegionBytesRead();

  printf("IPC: %f, L2 Cache Hit Ratio: %f, L3 Cache Hit Ratio: %f, Bytes Read: %ld\n", ipc, l2_hit, l3_hit, bytes);

  delete region;
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
