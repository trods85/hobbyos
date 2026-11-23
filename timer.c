#include "timer.h"
#include "ports.h"

#define PIT_HZ 1193182u
#define CALIBRATE_COUNT 11932u
#define FALLBACK_TSC_PER_MS 2000000ull

static uint64_t tsc_per_ms = FALLBACK_TSC_PER_MS;
static uint64_t tsc_start;

uint64_t timer_rdtsc(void){
	uint32_t lo, hi;
	__asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32)|lo;
}

void timer_init(void){
	uint8_t p61 = inb(0x61);

	outb(0x61, p61 & 0xFC);
	outb(0x43, 0xB0);
	outb(0x42, CALIBRATE_COUNT & 0xFF);
	outb(0x42, (CALIBRATE_COUNT >> 8) & 0xFF);
	outb(0x61, (p61 & 0xFC)| 0x01);

	uint64_t t0 = timer_rdtsc();
	uint32_t guard = 10000000u;
	while(!(inb(0x61)&0x20) && --guard){};
	uint64_t t1 = timer_rdtsc();

	outb(0x61, p61);

	if( guard != 0 ) {
		uint64_t per_ms = (t1 - t0) / 10;
		if( per_ms !=0 ) tsc_per_ms = per_ms;
	}
	tsc_start = timer_rdtsc();
}

uint64_t timer_ms(void){
	return(timer_rdtsc() - tsc_start)/tsc_per_ms;
}
