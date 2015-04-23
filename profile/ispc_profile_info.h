/** @file ispc_profile_info.h

    @brief Header file for representation of profiling information.
*/

#ifndef ISPC_PROFILE_INFO_H
#define ISPC_PROFILE_INFO_H

#include <stdint.h>
#include <string>

typedef struct {
  int line; 
  std::string file; 
  uint64_t mask;
} ISPCProfileInfo;

#endif /* ISPC_PROFILE_INFO_H */
