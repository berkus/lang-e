#if 0
long cvi2l(int x) { return x; }
unsigned cvi2u(int x) { return x; }
unsigned long cvi2ul(int x) { return x; }

int cvu2i(unsigned x) { return x; }
long cvu2l(unsigned x) { return x; }
unsigned long cvu2ul(unsigned x) { return x; }

unsigned cvul2u(unsigned long x) { return x; }
long cvul2l(unsigned long x) { return x; }
void * cvul2p(unsigned long x) { return (void *)x; }

unsigned long cvp2ul(void * x) { return (unsigned long)x; }

int cvl2i(long x) { return x; }
unsigned cvl2u(long x) { return x; }
unsigned long cvl2ul(long x) { return x; }
float cvi2f(int f) { return f; }
double cvi2d(int f) { return f; }


int cvf2i(float f) { return f; }
int cvd2i(double f) { return f; }

double cvf2d(float f) { return f; }
float cvd2f(double f) { return f; }

#endif
#if 0
double cvul2d(unsigned long x) { return x; }
float cvul2f(unsigned long x) { return x; }
int cvul2i(unsigned long x) { return x; }
long cvf2l(float f) { return f; }
long cvd2l(double f) { return f; }
#endif
float cvl2f(long f) { return f; }
double cvl2d(long f) { return f; }
