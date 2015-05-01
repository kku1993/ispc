/**
  * @file profile_ctx.cpp
  * @brief Utility functions and data structures to keep track of the current 
  *   profiling task.
  */

#include "profile_ctx.h"

////////////////////////////////////////////
// ProfileRegion
////////////////////////////////////////////
ProfileRegion::ProfileRegion(const char *note, int start_line, int end_line, 
    int task, uint64_t mask, SystemCounterState state) {
  this->note = note;
  this->start_line = start_line;
  this->end_line = end_line;
  this->task = task;
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
