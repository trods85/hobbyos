#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Calibrate CPU timestamp counter against PIT (takes ~ 10 ms)
void timer_init(void);

uint64_t timer_rdtsc(void);

uint64_t timer_ms(void);

#endif // TIME_H
