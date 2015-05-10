- Present new interface, `ISPCProfile`, to maintain backward compabtibility
  with existing `ISPCInstrument` function.
- Compile with `--profile`
  - No special includes needed: only need the header file generated for your ISPC file.
  - Macros
    - Fine grain control of what to measure
    - Can control how detailed the profiler should be
- Put the provided macros around calls to ISPC functions in cpp file.
- Handling `if` regions:
  - `else if` is treated like `if`. So if the code has the structure `if ... else if ... else ...`, it will be treated as 2 separate `if`, the first one without an `else` clause and the second one with. 
  - Lane usage for each case can be determined from the line number (ie: smaller line number is the true case)

TODO
====
- Add handler to clean up PCM on exit.
- Allow calls to ProfileInit from within ISPC.
- User program must be compiled with `-lpthread`
- Get extensive list of ispc library files and exclude them from the profiler.

Profile Context vs Region
=========================
- Context:
  - Only 1 context per task
  - Can be called in a nested fashion
  - Initialized when the user uses the provided macro OR upon task launch
  - Holds all regions within a task that are being profiled

- Region:
  - Key points in an ISPC program where control flow can diverge
  - Minimum resolution of what the user can control to profile
  - Region types:
```
  IF 0x2
  LOOP 0x4
  FOREACH 0x8
  SWITCH 0x10
  FUNCTION 0x20
```

Internal API
============
- `ISPCProfileInit`
  - Initializes a new profile context.
  - Automatically called upon task lauch.
- `ISPCProfileComplete`
  - Terminates the profile context in the current task.
- `ISPCProfileStart`
  - Add a new profile region to the current profile context.
- `ISPCProfileEnd`
  - Ends the most recent profile region.
- `ISPCProfileUpdate`
  - Updates the current region. 
  - Ex: loop iteration, if branch, switch case

Setup
=====
- Install `cmake`
- Build `rapidjson`
```
cd rapidjson; mkdir build; cd build; ../cmake
```

External Libraries Used
=======================
- [Intel Performance Counter Monitor](https://software.intel.com/en-us/articles/intel-performance-counter-monitor)
- [raidjson](https://github.com/miloyip/rapidjson)
