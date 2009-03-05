extern int printf();

void v_flushcache(void *ptr, int nbytes) {
#if 0
     asm volatile("sync");
     asm volatile("cflush 0x08");
#endif
}
