/**
  * @file profile_ctx.cpp
  * @brief Utility functions and data structures to keep track of the current 
  *   profiling task.
  */

#include "profile_ctx.h"

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
ProfileRegion::ProfileRegion(const char *note, int start_line, int end_line, 
    int task, int total_num_lanes, uint64_t mask, SystemCounterState state) {
  this->region_note = note;
  this->start_line = start_line;
  this->end_line = end_line;
  this->task = task;
  this->total_num_lanes = total_num_lanes;
  this->initial_mask = mask;
  this->entry_sstate = state;
}

ProfileRegion::~ProfileRegion() {
}

void ProfileRegion::setId(rid_t id) {
  this->id = id;
}

void ProfileRegion::updateExitStatus(SystemCounterState state) {
  this->exit_sstate = state;
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

void ProfileRegion::updateLineMask(int line, uint64_t mask) {
  int lanes_used = lanesUsed(this->total_num_lanes, mask);

  LaneUsageMap::iterator it = this->laneUsageMap.find(line);
  if (it == this->laneUsageMap.end()) {
    // We haven't encountered this line before.
    this->laneUsageMap.insert(it, std::make_pair(line, 
        std::make_pair(this->total_num_lanes, lanes_used)));
  } else {
    std::pair<int, int> usage = it->second;
    int total_lanes = usage.first + this->total_num_lanes;
    int total_used_lanes = usage.second + lanes_used;
    this->laneUsageMap[line] = std::make_pair(total_lanes, total_used_lanes);
  }

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
