#include "c.h"

static void vstring ARGS((char *, char **));

/* MAXP: TODO: get rid of buffers that are too small; deal with long strings
   in stringf in a beter way than by just increasing buffer size  */

int outfd = 1;
int errfd = 2;
int dcgfd = 3;
int sssfd = 4;
int cccfd = 5;
int hhhfd = 6;

const int outfdIdx = 1;
const int errfdIdx = 2;
const int dcgfdIdx = 3;
const int sssfdIdx = 4;
const int cccfdIdx = 5;
const int hhhfdIdx = 6;

const int cbufIdx = 7;

static char buf1[16*1024]; 
static char buf2[512];
static char buf3[16*1024];      /* output buffers */
static char buf4[16*1024];      /* output buffers */
static char buf5[16*1024];
static char buf6[16*1024];

static char cbuf[16*1024];

enum { SAFETY=80 };		/* "Safety zone" between buffer limit + end */

static struct io {
     int fd;			/* file descriptor */
     char *bp;			/* buffer pointer */
     char *buffer;		/* buffer proper */
     char *limit;		/* high water limit */
} iob[] = {
     { 0, 0, 0, 0, },
     { 1, buf1, buf1, buf1 + sizeof buf1 - SAFETY },
     { 2, buf2, buf2, buf2 + sizeof buf2 - SAFETY },
     { 3, buf3, buf3, buf3 + sizeof buf3 - SAFETY },
     { 4, buf4, buf4, buf4 + sizeof buf4 - SAFETY },
     { 5, buf5, buf5, buf5 + sizeof buf5 - SAFETY },
     { 6, buf6, buf6, buf6 + sizeof buf6 - SAFETY },
     { 7, cbuf, cbuf, cbuf + sizeof cbuf - SAFETY }
}, *io[] = {
     &iob[0],			/* used by stringf */
     &iob[1],			/* output */
     &iob[2],	                /* standard error & other files (fprint) */
     &iob[3],			/* templates output */
     &iob[4],
     &iob[5],
     &iob[6],
     &iob[7],
};

int curfdIdx = 1;		/* current output file */

char *bp = buf1;		/* current output buffer pointer */

void sw2dasm() {
     outflush();
     eval.tEmitting = 1;
     IR = dIR;
     curfdIdx = dcgfdIdx;
     bp = buf3;

     /* `C-C */
     if (cbe.have)
	  cbacksuspend();
}

void sw2sasm() {
     outflush();
     eval.tEmitting = 0;
     IR = sIR;
     curfdIdx = outfdIdx;
     bp = buf1;

     /* `C-C */
     if (cbe.have)
	  cbackrestart();
}

void outsfdx(s, fdx) char *s; int fdx; {
     char *p;
     struct io *iop = io[fdx];

     p = iop->bp;
     if (strlen(s) + p >= iop->limit + SAFETY) {
	  write(iop->fd, iop->buffer, p - iop->buffer);
	  iop->bp = iop->buffer;
     }

     for (p = iop->bp; (*p = *s++) != 0; p++)
	  ;

     if (p > iop->limit) {
	  write(iop->fd, iop->buffer, p - iop->buffer);
	  iop->bp = iop->buffer;
     } else
	  iop->bp = p;
}

void outs(s) char *s; {
     char *p;

     for (p = bp; (*p = *s++) != 0; p++)
	  ;
     bp = p;
     if (bp > io[curfdIdx]->limit)
	  outflush();
}

void outsverbatim(s) char *s; {

     vstring(s, &bp);
     if (bp > io[curfdIdx]->limit)
	  outflush();
}

void print VARARGS((char *fmt, ...),(fmt, va_alist),char *fmt; va_dcl) { 
     va_list ap;

     va_init(ap, fmt);
     vprint(fmt, ap);
     va_end(ap);
}

/* outputInit - initialize output file descriptors */
void outputInit() {
     io[outfdIdx]->fd = outfd;
     io[errfdIdx]->fd = errfd;
     io[dcgfdIdx]->fd = dcgfd;
     io[cccfdIdx]->fd = cccfd;
     io[hhhfdIdx]->fd = hhhfd;
     io[sssfdIdx]->fd = sssfd;
}

/* fprint - formatted output to file descriptor f */
void fprint VARARGS((int f, char *fmt, ...),
		    (f, fmt, va_alist),char *fmt; va_dcl) {
     va_list ap;

     va_init(ap, fmt);
     vfprint(f, fmt, ap);
     va_end(ap);
}

/* outflush - flush output buffer */
void outflush() {
     struct io *iop = io[curfdIdx];

     assert(curfdIdx);
     if (bp > iop->buffer)
	  write(iop->fd, iop->buffer, bp - iop->buffer);
     bp = iop->bp = iop->buffer;
}

/* outflushfdx - flush buffer fdx */
void outflushfdx(fdx) int fdx; {
     struct io *iop = io[fdx];

     if (iop->bp > iop->buffer)
	  write(iop->fd, iop->buffer, iop->bp - iop->buffer);
     iop->bp = iop->buffer;
}

/* outclearfdx - clear buffer fdx */
void outclearfdx(fdx) int fdx; {
     struct io *iop = io[fdx];
     iop->bp = iop->buffer;
}

/* stringf - formatted output to a saved string */
char *stringf VARARGS((char *fmt, ...),(fmt, va_alist),char *fmt; va_dcl) {
     char buf[4096];
     char *oldbp;
     int oldfdIdx;
     va_list ap;

     va_init(ap, fmt);

     oldfdIdx = curfdIdx;
     oldbp = bp;
     curfdIdx = 0;

     bp = io[curfdIdx]->bp = io[curfdIdx]->buffer = buf;
     io[curfdIdx]->limit = buf + sizeof buf;
     vprint(fmt, ap);
     *bp = 0;

     bp = oldbp;
     curfdIdx = oldfdIdx;

     va_end(ap);
     return string(buf);
}

/* vfprint - formatted output to file descriptor f */
void vfprint(f, fmt, ap) int f; char *fmt; va_list ap; {
     int oldfdIdx;
     char *oldbp;

     assert(f>=0 && f<= 3);

     oldfdIdx = curfdIdx;
     oldbp = bp;

     curfdIdx = f;
     bp = io[curfdIdx]->bp;

     vprint(fmt, ap);
     outflush();

     bp = oldbp;
     curfdIdx = oldfdIdx;
}

/* vprint - formatted output to standard output */
void vprint(fmt, ap) char *fmt; va_list ap; {
     for (; *fmt; fmt++)
	  if (*fmt == '%')
	       switch (*++fmt) {
	       case 'e': { double x = va_arg(ap, double);
	       char buf[80];
	       sprintf(buf, "%e", x);
	       outs(buf);
	       } break;
	       case 'f': { float x = va_arg(ap, float);
	       char buf[80];
	       sprintf(buf, "%f", x);
	       outs(buf);
	       } break;
	       case 'p': { void* x = va_arg(ap, void*);
	       char buf[80];
	       sprintf(buf, "%p", x);
	       outs(buf);
	       } break;
	       case 'u': { unsigned int x = va_arg(ap, unsigned int);
	       char buf[80];
	       sprintf(buf, "%u", x);
	       outs(buf);
	       } break;
	       case 'c': { *bp++ = va_arg(ap, int); } break;
	       case 'd': { int n = va_arg(ap, int);
	       unsigned m;
	       char buf[25], *s = buf + sizeof buf;
	       *--s = 0;
	       if (n == INT_MIN)
		    m = (unsigned)INT_MAX + 1;
	       else if (n < 0)
		    m = -n;
	       else
		    m = n;
	       do
		    *--s = m%10 + '0';
	       while ((m /= 10) != 0);
	       if (n < 0)
		    *--s = '-';
	       outs(s);
	       } break;
	       case 'o': { unsigned n = va_arg(ap, unsigned);
	       char buf[25], *s = buf + sizeof buf;
	       *--s = 0;
	       do
		    *--s = (n&7) + '0';
	       while ((n >>= 3) != 0);
	       outs(s);
	       } break;
	       case 'x': { unsigned n = va_arg(ap, unsigned);
	       char buf[25], *s = buf + sizeof buf;
	       *--s = 0;
	       do
		    *--s = "0123456789abcdef"[n&0xf];
	       while ((n >>= 4) != 0);
	       outs(s);
	       } break;
	       case 's': { char *s = va_arg(ap, char *);
	       if (s)
		    outs(s);
	       } break;
	       case 'S': { char *s = va_arg(ap, char *);
	       int n = va_arg(ap, int);
	       if (s)
		    while (n-- > 0)
			 *bp++ = *s++;
	       } break;
	       case 'k': { int t = va_arg(ap, int);
	       static char *tokens[] = {
#define xx(a,b,c,d,e,f,g) g,
#define yy(a,b,c,d,e,f,g) g,
#include "token.h"
	       };
	       assert(tokens[t&0177]);
	       outs(tokens[t&0177]);
	       } break;
	       case 't': { Type ty = va_arg(ap, Type);
	       outtype(ty ? ty : voidtype);
	       } break;
	       case 'w': { Coordinate *p = va_arg(ap, Coordinate *);
	       if (p->file && *p->file)
		    print("%s:", p->file);
	       print("%d", p->y); } break;
	       default:  *bp++ = *fmt; break;
	       }
	  else if ((*bp++ = *fmt) == '\n' && bp > io[curfdIdx]->limit)
	       outflush();
}

static void vstring(char *in, char **out) {
     char c;

     while (c = *in++) {
	  switch (c) {
	  case '\a': **out = '\\'; (*out)++;
	       **out = 'a'; (*out)++;
	       break;
	  case '\b': **out = '\\'; (*out)++;
	       **out = 'b'; (*out)++;
	       break;
	  case '\f': **out = '\\'; (*out)++;
	       **out = 'f'; (*out)++;
	       break;
	  case '\n': **out = '\\'; (*out)++;
	       **out = 'n'; (*out)++;
	       break;
	  case '\r': **out = '\\'; (*out)++;
	       **out = 'r'; (*out)++;
	       break;
	  case '\t': **out = '\\'; (*out)++;
	       **out = 't'; (*out)++;
	       break;
	  case '\v': **out = '\\'; (*out)++;
	       **out = 'v'; (*out)++;
	       break;
	  case '\\': **out = '\\'; (*out)++;
	       **out = '\\'; (*out)++;
	       break;
	  case '\'': **out = '\\'; (*out)++;
	       **out = '\''; (*out)++;
	       break;
	  case '\"': **out = '\\'; (*out)++;
	       **out = '\"'; (*out)++;
	       break;
	  case '\?': **out = '\\'; (*out)++;
	       **out = '\?'; (*out)++; /* trigraphs */
	       break;
	  default: **out = c; (*out)++;
	  }
     }
     **out = 0;
}

char * cbufstring(void) {
     struct io *cbp = io[cccfdIdx];
     return (cbp->bp > cbp->buffer) ? string(cbp->buffer) : "";
}

void cbufflush(char *s) {
     /* Requires: cbe.emit == 1 && (s == 0 || cbe.flush == 1) */
     char *p;
     struct io *cbp = io[cbufIdx];

     assert(cbe.emit);
     if (s) {
	  assert(cbe.flush && strlen(s) + cbp->bp < cbp->limit + SAFETY);
	  for (p = cbp->bp; (*p = *s++) != 0; p++)
	       ;
	  cbp->bp = p;
     }

     if (cbp->bp > cbp->buffer) {
	  outsfdx(cbp->buffer, cccfdIdx);
	  cbp->bp = cbp->buffer;
     }
}

void cbufput(char *s) {
     /* Requires: cbe.flush == 1 && cbe.emit == 1 */
     char *p;
     struct io *cbp = io[cbufIdx];

     assert(cbe.emit && cbe.flush);
     cbufflush(0);

     assert(strlen(s) + cbp->bp < cbp->limit + SAFETY);
     for (p = cbp->bp; (*p = *s++) != 0; p++)
	  ;
     cbp->bp = p;
}

void cbufputc(char c) { 
     /* Requires: cbe.flush == 1 && cbe.emit == 1 */
     /* Making this function have an ansi header once triggered bug at 
	gen.c:778; the problem is now fixed */
     char x[3], *p, *s = x;
     struct io *cbp = io[cbufIdx];

     assert(cbe.emit && cbe.flush);
     cbufflush(0);

     assert(cbp->bp < cbp->limit + SAFETY-2 );
     for (s[0] = c, s[1] = (c == ';') ? '\n' : ' ', s[2] = 0, p = cbp->bp; 
	  (*p = *s++) != 0; p++) ;
     cbp->bp = p;
}

void cbufputverbatim(char *s) {
     struct io *cbp = io[cbufIdx];

     assert(cbe.emit && cbe.flush);
     cbufflush(0);

     assert(strlen(s) + cbp->bp < cbp->limit + SAFETY);
     vstring(s, &(cbp->bp));
}

void cbufclear(void) {
     struct io *cbp = io[cbufIdx];
     cbp->bp = cbp->buffer;
}

void cbufflushon(void) {
     assert(!cbe.flush);
     cbe.flush = 1;
}

void cbufflushoff(void) {
     assert(cbe.flush);
     cbe.flush = 0;
}

void cbacksuspend(void) {
     assert(cbe.have && cbe.emit);
     cbufclear();
     cbe.emit = 0;
}

void cbackrestart(void) {
     assert(cbe.have && !cbe.emit);
     cbe.emit = 1;
}
