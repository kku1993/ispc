/**
  * @file profile_ctx.cpp
  * @brief Utility functions and data structures to keep track of the current 
  *   profiling task.
  */

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
ProfileRegion::ProfileRegion(const char *note, int region_type, int start_line,
    int end_line, int task, int total_num_lanes, uint64_t mask, 
    SystemCounterState state) {
  this->region_note = note;
  this->region_type = region_type;
  this->start_line = start_line;
  this->end_line = end_line;
  this->task = task;
  this->total_num_lanes = total_num_lanes;
  this->initial_mask = mask;
  this->entry_sstate = state;
}

ProfileRegion::~ProfileRegion() {
  this->laneUsageMap.clear();
}

void ProfileRegion::setId(rid_t id) {
  this->id = id;
}

void ProfileRegion::updateExitStatus(SystemCounterState state) {
  this->exit_sstate = state;
}

void ProfileRegion::updateEndLine(int end_line) {
  // Both end line provided to the constructor and the end line obtained from 
  // ProfileEnd are not reliable, so we get the best estimate of the 2.
  this->end_line = MAX(end_line, this->end_line);
}

double ProfileRegion::getRegionIPC() {
  return getIPC(this->entry_sstate, this->exit_sstate);
}

double ProfileRegion::getRegionL3HitRatio() {
  return getL3CacheHitRatio(this->entry_sstate, this->exit_sstate);
}

double ProfileRegion::getRegionL2HitRatio() {
  return getL2CacheHitRatio(this->entry_sstate, this->exit_sstate);
}

uint64_t ProfileRegion::getRegionBytesRead() {
  return getBytesReadFromMC(this->entry_sstate, this->exit_sstate);
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

void ProfileRegion::updateLineMask(int line, uint64_t mask) {
  int lanes_used = lanesUsed(this->total_num_lanes, mask);
  bool is_full_mask = lanes_used == this->total_num_lanes;

  // Update lane usage map.
  insertUsageMap(this->laneUsageMap, line, this->total_num_lanes, lanes_used);

  // Update full mask map.
  insertUsageMap(this->fullMaskMap, line, 1, is_full_mask ? 1 : 0);
}

std::string ProfileRegion::outputJSON() {
  const char *base = 
    "{"
      "\"region_id\":0,"
      "\"region_type\":0,"
      "\"start_line\":0,"
      "\"end_line\":0,"
      "\"task\":0,"
      "\"total_num_lanes\":0,"
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
  d["start_line"].SetInt(this->start_line);
  d["end_line"].SetInt(this->end_line);
  d["task"].SetInt(this->task);
  d["total_num_lanes"].SetInt(this->total_num_lanes);
  d["initial_mask"].SetUint64(this->initial_mask);
  d["ipc"].SetDouble(getRegionIPC());
  d["l2_hit"].SetDouble(getRegionL2HitRatio());
  d["l3_hit"].SetDouble(getRegionL3HitRatio());
  d["bytes_read"].SetDouble(getRegionBytesRead());

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
ProfileContext::ProfileContext() {
  this->region_id_counter = 0;
}

ProfileContext::~ProfileContext() {
}

// Adds a new profile region, which becomes the most recent profile region.
void ProfileContext::pushRegion(ProfileRegion *r) {
  if (r == NULL)
    return;

  rid_t id = this->region_id_counter++;
  r->setId(id);
  this->regions.push(r);
}

// Removes the most recent profile region.
// Return the JSON for the region.
ProfileRegion *ProfileContext::popRegion() {
  if (this->regions.empty())
    return NULL;

  ProfileRegion *r = this->regions.top();
  this->regions.pop();

  return r;
}

// Update the most recent profile region.
void ProfileContext::updateCurrentRegion(const char *note, int line, 
    uint64_t mask) {
  (void) note;
  if (this->regions.empty())
    return;

  ProfileRegion *r = this->regions.top();
  r->updateLineMask(line, mask);
}
