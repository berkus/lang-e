#include "ssdump.h"

void ssd_trace_on (void) { /* asm volatile("dbeg 0x1"); */ }
void ssd_trace_off (void) { /* asm volatile("dend 0x1"); */ }

void ssd_stats_on (void) { /* asm volatile("dbeg 0x2"); */ }
void ssd_stats_off (void) { /* asm volatile("dend 0x2"); */ }
