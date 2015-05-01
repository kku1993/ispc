- Present new interface, `ISPCProfile`, to maintain backward compabtibility
  with existing `ISPCInstrument` function.
- Compile with `--profile`
  - No special includes needed: only need the header file generated for your ISPC file.
- Put the provided macros around calls to ISPC functions in cpp file.

TODO
====
- Allow calls to ProfileInit from within ISPC.
- Add verbose flag support 
