/**
 *  @file profile_ctx.h
 *  @brief Header file for profile context.
 */
#ifndef _PROFILE_CTX_H_
#define _PROFILE_CTX_H_

#include <cstdint>
#include <map>
#include <stack>
#include <string>
#include <list>

#include "intel_pcm/cpucounters.h"

// Region id type.
typedef uint64_t rid_t;

// Lane usage map type. Maps line number to 
// (total number of lanes available, lanes actually used)
typedef std::map<int, std::pair<int, int> > LaneUsageMap;

// Struct to keep track each profiling region surrounded by 
// and ProfileStart/ProfileEnd.
class ProfileRegion{
  private:
    // Intel PCM state upon entry to this region.
    SystemCounterState entry_sstate;

    // Intel PCM state upon leaving this region.
    SystemCounterState exit_sstate;

    // Unique id of the region.
    rid_t id;

    const char *region_note;
    int region_type;
    int start_line;
    int end_line;
    int task;

    // Initial mask upon entering the region.
    uint64_t initial_mask;

    // Map line within a region to (total number of of lanes available when 
    // the line was run, total number of lanes that the line ran with).
    LaneUsageMap laneUsageMap;

    // Map line to (number of times the line was run, number of times the line 
    // was run with full mask)
    LaneUsageMap fullMaskMap;

  public:
    ProfileRegion(const char*, int, int, int, uint64_t, SystemCounterState);
    ~ProfileRegion();
    void setId(rid_t);
    void updateExitStatus(SystemCounterState);
    void updateEndLine(int);
    double getRegionIPC();
    double getRegionL3HitRatio();
    double getRegionL2HitRatio();
    uint64_t getRegionBytesRead();
    void updateLineMask(int line, uint64_t mask, int total_num_lanes);
    std::string outputJSON();
};

class ProfileContext{
  private:
    // Id of the task the context is in. Each task can only have at most 1 
    // context.
    int task_id;

    // Counter for assigning unique region ids.
    // The id is assigned in monotonically increasing order, so it can also 
    // be used to identify which region started first (useful when dealing with
    // nested regions in recursion)
    rid_t region_id_counter;

    // Profile regions are organized in a stack so all profiling information
    // is associated with the most recent profile region until the region has
    // ended.
    std::stack<ProfileRegion *> regions;

    const char *profile_name;
    int profile_line;
    int verbose;
    // Total number of available lanes.
    int total_num_lanes;

    // List to keep old regions that have left their scopes.
    std::list<ProfileRegion *> old_regions;

  public:
    ProfileContext(const char* name, int line, int num_lanes, int verbose, 
        int task_id);
    ~ProfileContext();
    void outputProfile();
    void pushRegion(ProfileRegion *);
    void popRegion(SystemCounterState exit_state, int end_line);
    void updateCurrentRegion(const char *note, int line, uint64_t mask);
};

#endif /* _PROFILE_CTX_H_ */
