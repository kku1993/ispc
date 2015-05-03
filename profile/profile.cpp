#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <list>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#include "intel_pcm/cpucounters.h"
#include "profile_ctx.h"

extern "C" {
  void ISPCProfileInit(const char *fn, int line, int total_lanes, int verbose);
  void ISPCProfileComplete();
  void ISPCProfileStart(const char *note, int region_type, int start_line, 
      int end_line, int task, uint64_t mask);
  void ISPCProfileEnd(int end_line);
  void ISPCProfileIteration(const char *note, int line, int64_t mask);
  void ISPCProfileIf(const char *note, int line, int64_t mask);
}

// Indicate whether a profiler is running already.
// Avoid initializing the profiler multiple times.
static bool profile_running = false;

// Information of the ISPC function being profiled.
static const char *profile_name;
static int profile_line;

// Vector width that the program is compiled for.
static int num_lanes;

// Intel Performance Counter Monitor
static PCM *monitor;

// Context to keep track of the current profile region in scope.
static ProfileContext *ctx = new ProfileContext();

// List to keep outputs of regions that have left their scopes.
static std::list<std::string> region_json;

/*
static void mask_to_str(uint64_t mask, char *buffer) {
  for (int i = 0; i < num_lanes; i++) {
    buffer[i] = (mask >> (num_lanes - 1 - i)) & 0x1 ? '1' : '0';
  }
}
*/

void ISPCProfileInit(const char *file, int line, int total_lanes, int verbose) {
  if (profile_running)
    return;

  (void) verbose;
  profile_running = true;
  profile_name = file;
  profile_line = line;
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

  // Create output folder.
  struct tm *tm;
  time_t t;
  char date[128];
  char dir[128];
  time(&t);
  tm = localtime(&t);
  strftime(date, sizeof (date), "%Y%m%d%H%M%S", tm);
  sprintf(dir, "profile_%s", date);

  struct stat st;
  if (stat(dir, &st) == -1 && mkdir(dir, 0700) == -1) {
    printf("ERROR: Profiler failed to create directory %s\n", dir);
    region_json.clear();
    return;
  }

  // Open file for output.
  char outname[NAME_MAX + strlen(dir) + 100];
  memset(outname, '\0', sizeof (char) * (NAME_MAX + strlen(dir) + 100));
  sprintf(outname, "%s/%s.%d", dir, profile_name, profile_line);
  FILE *fp = fopen(outname, "w+");
  if (fp == NULL) {
    printf("ERROR: Profiler failed to open output file %s\n", outname);
    region_json.clear();
    return;
  }

  // Output json for each region.
  fprintf(fp, "[\n");
  while (!region_json.empty()) {
    char comma = region_json.size() == 1 ? ' ' : ',';
    fprintf(fp, "%s%c\n", region_json.front().c_str(), comma);
    region_json.pop_front();
  }
  fprintf(fp, "]");

  region_json.clear();
  profile_running = false;
}

void ISPCProfileStart(const char *note, int region_type, int start_line, 
    int end_line, int task, uint64_t mask) {
  // Get Intel performance monitor state
  SystemCounterState state = getSystemCounterState();

  ProfileRegion *region = new ProfileRegion(note, region_type, start_line, 
      end_line, task, num_lanes, mask, state);
  ctx->pushRegion(region);
}

void ISPCProfileEnd(int end_line) {
  SystemCounterState after_sstate = getSystemCounterState();

  // Remove the profile region from the profiling context.
  std::string json = ctx->popRegion(after_sstate, end_line);
  region_json.push_back(json); 
}

void ISPCProfileIteration(const char *note, int line, int64_t mask) {
  ctx->updateCurrentRegion(note, line, mask);
}

void ISPCProfileIf(const char *note, int line, int64_t mask) {
  ctx->updateCurrentRegion(note, line, mask);
}
