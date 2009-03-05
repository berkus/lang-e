#include "c.h"
extern Interface nullIR,   symbolicIR;
extern Interface mipsebIR, mipselIR;
extern Interface mipselIIR, mipselVIR;
extern Interface sslittleIIR, sslittleVIR;
extern Interface ssbigIIR, ssbigVIR;
extern Interface sparcIR,  solarisIR;
extern Interface sparcIIR, sparcVIR;
extern Interface i386linuxIR;
extern Interface i386IIR, i386VIR;
extern Interface x86IR;
extern Interface cIR;
Binding * binding;
Binding bindings[] = {
     { "symbolic",      &symbolicIR, NULL, NULL },
     { "mips-irix",     &mipsebIR,   NULL, NULL },
     { "mips-ultrix",   &mipselIR,   &mipselIIR, &mipselVIR },
     { "sslittle-sstrix",&mipselIR,  &sslittleIIR, &sslittleVIR },
     { "ssbig-sstrix",  &mipselIR,  &ssbigIIR, &ssbigVIR },
     { "sparc-sun",     &sparcIR,    &sparcIIR, &sparcVIR },
     { "sparc-solaris", &solarisIR,  &sparcIIR, &sparcVIR },
     { "i386-linux",	&i386linuxIR,&i386IIR, &i386VIR },
     { "x86-dos",       &x86IR,      NULL, NULL },
     { "c-mips",	&cIR,        &mipselIIR, &mipselVIR },
     { "c-sparc",	&cIR,	     &sparcIIR, &sparcVIR },
     { "c-sslittle",	&cIR,	     &sslittleIIR, &sslittleVIR },
     { "c-ssbig",	&cIR,	     &ssbigIIR, &ssbigVIR },
     { "c-i386",	&cIR,	     &i386IIR, &i386VIR },
     { "null",          &nullIR,     &nullIR, NULL },
     { NULL,            NULL,        NULL, NULL },
};
