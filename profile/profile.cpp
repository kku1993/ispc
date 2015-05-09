#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <pthread.h>

#include "intel_pcm/cpucounters.h"
#include "profile_ctx.h"
#include "profile_region_types.h"
#include "profile_flags.h"

extern "C" {
  void ISPCProfileInit(const char *fn, int line, int total_lanes, int flags);
  void ISPCProfileComplete();
  void ISPCProfileStart(const char *filename, int region_type, int start_line, 
      int end_line, int task, uint64_t mask);
  void ISPCProfileEnd(int region_type, int end_line);
  void ISPCProfileIteration(const char *note, int line, int64_t mask, 
      int region_type);
  void ISPCProfileIf(const char *note, int line, int64_t mask, 
      int region_type);

  ProfileContext *getContext(bool pop);
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

ProfileContext *getContext(bool pop) {
  pthread_t thread = pthread_self();

  pthread_mutex_lock(&ctx_map_lock);

  ProfileContext *ctx = ctx_map[thread];
  if (pop)
    ctx_map.erase(thread);

  // Clean up PCM 
  if (ctx_map.size() == 0)
    monitor->cleanup();

  pthread_mutex_unlock(&ctx_map_lock);

  return ctx;
}

void ISPCProfileInit(const char *file, int line, int total_lanes, int flags) {
  if (strcmp(file, "stdlib.ispc") == 0)
    return;

  pthread_t thread = pthread_self();

  pthread_mutex_lock(&ctx_map_lock);

  // Initialize Intel performance monitor 
  if (ctx_map.size() == 0) {
    monitor = PCM::getInstance();

    PCM::ErrorCode err = monitor->program();
    if (err != PCM::Success) {
      fprintf(stderr, "PCM init failed [error = %d].\n", err);
      exit(1);
    }
  }

  ProfileContextMap::iterator it = ctx_map.find(thread);
  if (it == ctx_map.end()) {
    // Create new context.
    ProfileContext *ctx = new ProfileContext(file, line, total_lanes, flags,
      task_id_counter++);
    ctx_map[thread] = ctx;
  } 

  pthread_mutex_unlock(&ctx_map_lock);
}

void ISPCProfileComplete() {
  ProfileContext *ctx = getContext(true);

  if (ctx == NULL) {
    //fprintf(stderr, "Error: Profile complete without context.\n");
    return;
  }

  ctx->outputProfile();
  delete ctx;
}

void ISPCProfileStart(const char *filename, int region_type, int start_line, 
    int end_line, int task, uint64_t mask) { 

  // Don't profile library functions.
  if (strcmp(filename, "stdlib.ispc") == 0)
    return;
  
  // Using task id assigned to context to identify a region's task instead.
  (void) task;

  ProfileContext *ctx = getContext(false);

  if (ctx == NULL) {
    //fprintf(stderr, "Error: Profile region started without context.\n");
    return;
  }

  // Skip this type of region if the user doesn't want to profile it.
  int flags = ctx->getFlags();
  if ((flags & region_type) == 0) {
    return;
  }

  // Get Intel performance monitor state if the user requested it.
  SystemCounterState *state;
  SystemCounterState s;
  if ((flags & ISPC_PROFILE_PCM) == 0) {
    state = NULL;
  } else {
    s = getSystemCounterState();
    state = &s;
  }

  ctx->pushRegion(filename, region_type, start_line, end_line, mask, state);
}

void ISPCProfileEnd(int region_type, int end_line) {
  (void) region_type;

  ProfileContext *ctx = getContext(false);

  if (ctx == NULL) {
    //fprintf(stderr, "Error: Profile region ended without context.\n");
    return;
  }

  // Skip this type of region if the user doesn't want to profile it.
  int flags = ctx->getFlags();
  if ((flags & region_type) == 0) {
    return;
  }

  // Get Intel performance monitor state if the user requested it.
  SystemCounterState *state;
  SystemCounterState s;
  if ((flags & ISPC_PROFILE_PCM) == 0) {
    state = NULL;
  } else {
    s = getSystemCounterState();
    state = &s;
  }

  // Remove the profile region from the profiling context.
  ctx->popRegion(state, end_line);
}

void ISPCProfileIteration(const char *note, int line, int64_t mask, 
    int region_type) {
  ProfileContext *ctx = getContext(false);

  if (ctx == NULL) {
    //fprintf(stderr, "Error: Profile iteration without context.\n");
    return;
  }

  // Skip this type of region if the user doesn't want to profile it.
  int flags = ctx->getFlags();
  if ((flags & region_type) == 0) {
    return;
  }

  ctx->updateCurrentRegion(note, line, mask);
}

void ISPCProfileIf(const char *note, int line, int64_t mask, int region_type) {
  ProfileContext *ctx = getContext(false);

  if (ctx == NULL) {
    //fprintf(stderr, "Error: Profile if statement without context.\n");
    return;
  }

  // Skip this type of region if the user doesn't want to profile it.
  int flags = ctx->getFlags();
  if ((flags & region_type) == 0) {
    return;
  }

  ctx->updateCurrentRegion(note, line, mask);
}
