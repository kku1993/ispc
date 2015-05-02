/**
 *  @file profile_ctx.h
 *  @brief Header file for profile context.
 */
#include <map>
#include <stack>
#include <stdint.h>

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
    int start_line;
    int end_line;
    int task;
    int total_num_lanes;

    // Initial mask upon entering the region.
    uint64_t initial_mask;

    // Map line within a region to lane usage information.
    LaneUsageMap laneUsageMap;

  public:
    ProfileRegion(const char*, int, int, int, int, uint64_t, 
        SystemCounterState);
    ~ProfileRegion();
    void setId(rid_t);
    void updateExitStatus(SystemCounterState);
    double getRegionIPC();
    double getRegionL3HitRatio();
    double getRegionL2HitRatio();
    uint64_t getRegionBytesRead();
    void updateLineMask(int line, uint64_t mask);
};

class ProfileContext{
  private:
    // Counter for assigning unique region ids.
    rid_t region_id_counter;

    // Profile regions are organized in a stack so all profiling information
    // is associated with the most recent profile region until the region has
    // ended.
    std::stack<ProfileRegion *> regions;

  public:
    ProfileContext();
    ~ProfileContext();
    void pushRegion(ProfileRegion *);
    ProfileRegion *popRegion();
    void updateCurrentRegion(const char *note, int line, uint64_t mask);
};
