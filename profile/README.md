- Present new interface, `ISPCProfile`, to maintain backward compabtibility
  with existing `ISPCInstrument` function.
- Compile with `--profile`
- Put the provided macros around calls to ISPC functions in cpp file.

TODO
====
- Allow calls to ProfileInit from within ISPC.
- Add verbose flag support 
