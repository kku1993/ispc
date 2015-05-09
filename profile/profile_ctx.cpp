/**
  * @file profile_ctx.cpp
  * @brief Utility functions and data structures to keep track of the current 
  *   profiling task.
  */

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#include "profile_ctx.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

using namespace rapidjson;

////////////////////////////////////////////
// Util
////////////////////////////////////////////
static int lanesUsed(int total_num_lanes, uint64_t mask) {
  int lanes_used = 0;
  for (int i = 0; i < total_num_lanes; i++) {
    lanes_used += (mask >> i) & 0x1;
  }
  return lanes_used;
}

////////////////////////////////////////////
// ProfileRegion
////////////////////////////////////////////
ProfileRegion::ProfileRegion(const char *fn, int region_type, int start_line,
    int end_line, uint64_t mask) {
  this->file_name = fn;
  this->region_type = region_type;
  this->start_line = start_line;
  this->end_line = end_line;
  this->initial_mask = mask;

  this->num_entry = 0;
  this->avg_ipc = 0;
  this->avg_l2_hit = 0;
  this->avg_l3_hit = 0;
  this->avg_bytes_read = 0;
}

ProfileRegion::~ProfileRegion() {
  this->laneUsageMap.clear();
}

void ProfileRegion::setId(rid_t id) {
  this->id = id;
}

void ProfileRegion::enterRegion(SystemCounterState *state) {
  if (state != NULL)
    memcpy(&this->entry_sstate, state, sizeof (SystemCounterState));

  this->num_entry += 1;
}

void ProfileRegion::exitRegion(SystemCounterState *state, int end_line) {
  // Both end line provided to the constructor and the end line obtained from 
  // ProfileEnd are not reliable, so we get the best estimate of the 2.
  this->end_line = MAX(end_line, this->end_line);

  // Update PCM stats.
  if (state != NULL) {
    double ipc = this->avg_ipc * (this->num_entry - 1) + getRegionIPC(*state);
    double l2 = this->avg_l2_hit * (this->num_entry - 1)
        + getRegionL2HitRatio(*state);
    double l3 = this->avg_l3_hit * (this->num_entry - 1)
        + getRegionL3HitRatio(*state);
    double bytes_read = this->avg_bytes_read * (this->num_entry - 1)
        + getRegionBytesRead(*state);

    this->avg_ipc = ipc / this->num_entry;
    this->avg_l2_hit = l2 / this->num_entry;
    this->avg_l3_hit = l3 / this->num_entry;
    this->avg_bytes_read = bytes_read / this->num_entry;
  }
}

int ProfileRegion::getStartLine() {
  return this->start_line;
}

int ProfileRegion::getRegionType() {
  return this->region_type;
}

double ProfileRegion::getRegionIPC(SystemCounterState exit_state) {
  return getIPC(this->entry_sstate, exit_state);
}

double ProfileRegion::getRegionL3HitRatio(SystemCounterState exit_state) {
  return getL3CacheHitRatio(this->entry_sstate, exit_state);
}

double ProfileRegion::getRegionL2HitRatio(SystemCounterState exit_state) {
  return getL2CacheHitRatio(this->entry_sstate, exit_state);
}

uint64_t ProfileRegion::getRegionBytesRead(SystemCounterState exit_state) {
  return getBytesReadFromMC(this->entry_sstate, exit_state);
}

static void insertUsageMap(LaneUsageMap &m, int line, int dtotal, int dval) {
  LaneUsageMap::iterator it = m.find(line);
  if (it == m.end()) {
    // We haven't encountered this line before.
    m.insert(it, std::make_pair(line, std::make_pair(dtotal, dval)));
  } else {
    std::pair<int, int> usage = it->second;
    int total = usage.first + dtotal;
    int val = usage.second + dval;
    m[line] = std::make_pair(total, val);
  }
}

// total_num_lanes = SIMD width of the machine
void ProfileRegion::updateLineMask(int line, uint64_t mask, 
    int total_num_lanes) {
  int lanes_used = lanesUsed(total_num_lanes, mask);

  // Update lane usage map.
  insertUsageMap(this->laneUsageMap, line, total_num_lanes, lanes_used);

  // Update full mask map.

  // Is full mask is based on the number of lanes available to the region 
  // upon entry into the region. 
  // To handle unmasked regions, we take the max of the initial mask usage
  // and the current usage. Current usage can only be greater than the initial
  // usage in unmasked regions.
  int lanes_available = MAX(lanesUsed(total_num_lanes, this->initial_mask), 
      lanes_used);
  bool is_full_mask = (lanes_used == lanes_available);

  insertUsageMap(this->fullMaskMap, line, 1, is_full_mask ? 1 : 0);
}

std::string ProfileRegion::outputJSON() {
  const char *base = 
    "{"
      "\"region_id\":0,"
      "\"region_type\":0,"
      "\"file_name\":\"\","
      "\"start_line\":0,"
      "\"end_line\":0,"
      "\"initial_mask\":0,"
      "\"lane_usage\":[],"
      "\"full_mask_percentage\": [],"
      "\"ipc\":0,"
      "\"l2_hit\":0,"
      "\"l3_hit\":0,"
      "\"bytes_read\":0"
    "}";

  Document d;
  d.Parse(base);

  d["region_id"].SetUint64(this->id);
  d["region_type"].SetInt(this->region_type);
  d["file_name"].SetString(StringRef(this->file_name));
  d["start_line"].SetInt(this->start_line);
  d["end_line"].SetInt(this->end_line);
  d["initial_mask"].SetUint64(this->initial_mask);
  d["ipc"].SetDouble(this->avg_ipc);
  d["l2_hit"].SetDouble(this->avg_l2_hit);
  d["l3_hit"].SetDouble(this->avg_l3_hit);
  d["bytes_read"].SetDouble(this->avg_bytes_read);

  // Add list of lane usage by line number.
  Value &lane_usage = d["lane_usage"];
  Document::AllocatorType &allocator = d.GetAllocator();
  for (LaneUsageMap::iterator it = this->laneUsageMap.begin(); 
      it != this->laneUsageMap.end(); ++it) {
    Value line(kObjectType);

    double percent = it->second.second/double(it->second.first) * 100;

    line.AddMember("line", it->first, allocator);
    line.AddMember("percent", percent, allocator);

    lane_usage.PushBack(line, allocator); 
  }

  // Add list of percent of full mask runs by line number.
  Value &full_mask = d["full_mask_percentage"];
  for (LaneUsageMap::iterator it = this->fullMaskMap.begin(); 
      it != this->fullMaskMap.end(); ++it) {
    Value line(kObjectType);

    double percent = it->second.second/double(it->second.first) * 100;

    line.AddMember("line", it->first, allocator);
    line.AddMember("percent", percent, allocator);

    full_mask.PushBack(line, allocator); 
  }
  
  // Stringify the DOM
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  d.Accept(writer);

  std::string str(buffer.GetString());
  return str;
}

////////////////////////////////////////////
// ProfileContext
////////////////////////////////////////////
ProfileContext::ProfileContext(const char* name, int line, int num_lanes, 
    int verbose, int task_id) {
  this->region_id_counter = 0;

  (void) verbose;
  this->verbose = verbose;
  this->profile_name = name;
  this->profile_line = line;
  this->total_num_lanes = num_lanes;
  this->task_id = task_id;
}

ProfileContext::~ProfileContext() {
}

void ProfileContext::outputProfile() {
  // Don't output if there are no region has completed profiling in this 
  // context.
  if (this->old_regions.size() == 0) {
    return;
  }

  // Create output folder.
  const char *dir = "profile_results";
  struct stat st;
  if (stat(dir, &st) == -1 && mkdir(dir, 0700) == -1) {
    printf("ERROR: Profiler failed to create directory %s\n", dir);
    this->old_regions.clear();
    return;
  }

  // Get current time.
  struct tm *tm;
  time_t t;
  char date[128];
  time(&t);
  tm = localtime(&t);
  strftime(date, sizeof (date), "%Y%m%d%H%M%S", tm);

  // Open file for output.
  char outname[NAME_MAX + strlen(dir) + 100];
  memset(outname, '\0', sizeof (char) * (NAME_MAX + strlen(dir) + 100));
  sprintf(outname, "%s/%s.line%d.task%d.%s", dir, this->profile_name, 
      this->profile_line, this->task_id, date);
  FILE *fp = fopen(outname, "w+");
  if (fp == NULL) {
    printf("ERROR: Profiler failed to open output file %s\n", outname);
    this->old_regions.clear();
    return;
  }

  // Output json for context information
  fprintf(fp, "{"
      "\"file\":\"%s\","
      "\"line\":%d,"
      "\"total_num_lanes\":%d,"
      "\"task\":%d,", 
      this->profile_name, this->profile_line, 
      this->total_num_lanes, this->task_id);

  // Output json for each region.
  fprintf(fp, "\"regions\": [\n");
  for (RegionMap::iterator it = this->old_regions.begin(); 
      it != this->old_regions.end(); ++it) {
    char comma = this->old_regions.end() == it ? ' ' : ',';
    ProfileRegion *r = it->second;
    fprintf(fp, "%s%c\n", r->outputJSON().c_str(), comma);
  }
  fprintf(fp, "]");
  fprintf(fp, "}");

  this->old_regions.clear();
}

// Adds a new profile region, which becomes the most recent profile region.
void ProfileContext::pushRegion(const char *filename, int region_type, 
    int start_line, int end_line, uint64_t mask, SystemCounterState *state) {

  ProfileRegion *r;

  // Recyle old region.
  RegionMap::iterator it = this->old_regions.find(
      std::make_pair(start_line, region_type));
  if (it != this->old_regions.end()) {
    r = it->second;
  } else {
    r = new ProfileRegion(filename, region_type, start_line, end_line, mask);
    rid_t id = this->region_id_counter++;
    r->setId(id);
  }

  r->enterRegion(state);

  this->regions.push(r);
}

// Removes the most recent profile region.
// Return the JSON for the region.
void ProfileContext::popRegion(SystemCounterState *exit_state, int end_line) {
  if (this->regions.empty())
    return;

  ProfileRegion *r = this->regions.top();
  this->regions.pop();

  r->exitRegion(exit_state, end_line);

  int start_line = r->getStartLine();
  int region_type = r->getRegionType();
  this->old_regions[std::make_pair(start_line, region_type)] = r;
}

// Update the most recent profile region.
void ProfileContext::updateCurrentRegion(const char *note, int line, 
    uint64_t mask) {
  (void) note;
  if (this->regions.empty())
    return;

  ProfileRegion *r = this->regions.top();
  r->updateLineMask(line, mask, this->total_num_lanes);
}
