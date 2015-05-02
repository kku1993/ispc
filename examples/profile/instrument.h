#ifndef _INSTRUMENT_H_
#define _INSTRUMENT_H_

#include <stdint.h>

extern "C" {
  void ISPCInstrument(const char *fn, const char *note, int line, 
      uint64_t mask);
}

void ISPCPrintInstrument();

#endif /* _INSTRUMENT_H_ */
