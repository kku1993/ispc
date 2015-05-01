/**
 *  @file profile_ctx.h
 *  @brief Header file for profile context.
 */
#include <stdint.h>
#include <stack>

#include "intel_pcm/cpucounters.h"

typedef uint64_t rid_t;

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

    const char *note;
    int start_line;
    int end_line;
    int task;
    uint64_t initial_mask;

  public:
    ProfileRegion(const char*, int, int, int, uint64_t, SystemCounterState);
    ~ProfileRegion();
    void setId(rid_t);
    void updateExitStatus(SystemCounterState);
    double getRegionIPC();
    double getRegionL3HitRatio();
    double getRegionL2HitRatio();
    uint64_t getRegionBytesRead();
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
};
