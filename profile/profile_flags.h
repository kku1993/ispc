/**
 *  @file profile_flags.h
 *  @brief Defines constants to control what to profile.
 */

#ifndef _PROFILE_FLAGS_H_
#define _PROFILE_FLAGS_H_

#define ISPC_PROFILE_IF 0x2
#define ISPC_PROFILE_LOOP 0x4
#define ISPC_PROFILE_FOREACH 0x8
#define ISPC_PROFILE_SWITCH 0x10
#define ISPC_PROFILE_FUNCTION 0x20
#define ISPC_PROFILE_PCM 0x40
#define ISPC_PROFILE_ALL ( \
  ISPC_PROFILE_IF | \
  ISPC_PROFILE_LOOP | ISPC_PROFILE_FOREACH | \
  ISPC_PROFILE_SWITCH | ISPC_PROFILE_FUNCTION | \
  ISPC_PROFILE_PCM \
  )
#define ISPC_PROFILE_ALL_NO_PCM ( \
  ISPC_PROFILE_IF | \
  ISPC_PROFILE_LOOP | ISPC_PROFILE_FOREACH | \
  ISPC_PROFILE_SWITCH | ISPC_PROFILE_FUNCTION \
  )

#endif /* _PROFILE_FLAGS_H_ */
