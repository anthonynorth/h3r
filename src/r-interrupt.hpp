#pragma once

#include <exception>

// sentinel exception for user interrupts
struct interrupt_error : std::exception {};

// interrupt pending & interrupts aren't suspended
bool interrupt_requested();

// throws interrupt_error if interrupt requested
void check_interrupt();
