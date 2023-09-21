#define R_NO_REMAP

#include "r-interrupt.hpp"
#include <Rinternals.h>
// R_interrupts_suspended & R_interrupts_pending
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>

// UserBreak
#ifdef _WIN32
#include <Rembedded.h>
#else
constexpr int UserBreak = 0;
#endif

bool interrupt_requested() {
  return (UserBreak || R_interrupts_pending) && !R_interrupts_suspended;
}

void check_interrupt() {
  if (interrupt_requested()) throw interrupt_error();
}
