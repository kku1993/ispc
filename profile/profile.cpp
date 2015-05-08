#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <pthread.h>

#include "intel_pcm/cpucounters.h"
#include "profile_ctx.h"

extern "C" {
  void ISPCProfileInit(const char *fn, int line, int total_lanes, int verbose);
  void ISPCProfileComplete();
  void ISPCProfileStart(const char *filename, int region_type, int start_line, 
      int end_line, int task, uint64_t mask);
  void ISPCProfileEnd(int end_line);
  void ISPCProfileIteration(const char *note, int line, int64_t mask);
  void ISPCProfileIf(const char *note, int line, int64_t mask);
}

// Map thread id to the current profile region in scope.
typedef std::map<pthread_t, ProfileContext *> ProfileContextMap;
static ProfileContextMap ctx_map;

// Mutex to guard the map of ProfileContexts
static pthread_mutex_t ctx_map_lock = PTHREAD_MUTEX_INITIALIZER;

// Counter to assign task id to contexts.
static int task_id_counter = 0;

// Intel performance monitor
static PCM *monitor;

/*
static void mask_to_str(uint64_t mask, char *buffer) {
  for (int i = 0; i < num_lanes; i++) {
    buffer[i] = (mask >> (num_lanes - 1 - i)) & 0x1 ? '1' : '0';
  }
}
*/

static ProfileContext *getContext(bool pop) {
  pthread_t thread = pthread_self();

  pthread_mutex_lock(&ctx_map_lock);

  ProfileContext *ctx = ctx_map[thread];
  if (pop)
    ctx_map.erase(thread);

  pthread_mutex_unlock(&ctx_map_lock);

  return ctx;
}

void ISPCProfileInit(const char *file, int line, int total_lanes, int verbose) {
  pthread_t thread = pthread_self();

  pthread_mutex_lock(&ctx_map_lock);

  // Initialize Intel performance monitor 
  if (ctx_map.size() == 0) {
    monitor = PCM::getInstance();
    monitor->program(PCM::DEFAULT_EVENTS, NULL);

    PCM::ErrorCode err = monitor->program();
    if (err != PCM::Success) {
      // TODO report pcm failed
      printf("PCM init failed [error = %d].\n", err);
    }
  }

  ProfileContextMap::iterator it = ctx_map.find(thread);
  if (it == ctx_map.end()) {
    // Create new context.
    ProfileContext *ctx = new ProfileContext(file, line, total_lanes, verbose,
      task_id_counter++);
    ctx_map[thread] = ctx;
  } 

  pthread_mutex_unlock(&ctx_map_lock);
}

void ISPCProfileComplete() {
  ProfileContext *ctx = getContext(true);
  ctx->outputProfile();
  delete ctx;
}

void ISPCProfileStart(const char *filename, int region_type, int start_line, 
    int end_line, int task, uint64_t mask) { 
  
  // Using task id assigned to context to identify a region's task instead.
  (void) task;

  // Get Intel performance monitor state
  SystemCounterState state = getSystemCounterState();

  ProfileRegion *region = new ProfileRegion(filename, region_type, start_line, 
      end_line, mask, state);

  ProfileContext *ctx = getContext(false);

  ctx->pushRegion(region);
}

void ISPCProfileEnd(int end_line) {
  SystemCounterState after_sstate = getSystemCounterState();

  ProfileContext *ctx = getContext(false);

  // Remove the profile region from the profiling context.
  ctx->popRegion(after_sstate, end_line);
}

void ISPCProfileIteration(const char *note, int line, int64_t mask) {
  ProfileContext *ctx = getContext(false);
  ctx->updateCurrentRegion(note, line, mask);
}

void ISPCProfileIf(const char *note, int line, int64_t mask) {
  ProfileContext *ctx = getContext(false);
  ctx->updateCurrentRegion(note, line, mask);
}
