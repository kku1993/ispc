- Present new interface, `ISPCProfile`, to maintain backward compabtibility
  with existing `ISPCInstrument` function.
- Compile with `--profile`
  - No special includes needed: only need the header file generated for your ISPC file.
  - Macros
    - Fine grain control of what to measure
    - Can control how detailed the profiler should be
- Put the provided macros around calls to ISPC functions in cpp file.

TODO
====
- Allow calls to ProfileInit from within ISPC.
- Test compability with pthreads and processes.
- Allow user to decide what kinds of regions to log.
- Explain the region design concept.

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
