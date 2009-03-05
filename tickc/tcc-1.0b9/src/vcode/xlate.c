/* Go between type strings and integer types. */
#include "vcode-internal.h"

/* go between string and internal type representation.  Capital
 * letters indicate an immediate.  NOTE: fallthroughs are intentional.  */
static inline char * xlatee(char *fmt, int *type) {
        unsigned mask;

        demand(fmt && fmt[0] != '\0', must give a valid format);

        if(fmt[0] != '%')
                v_fatal("bogus fmt: expecting `%%', got `%c'\n", fmt[0]);

        mask = 0;
        switch(fmt[1]) {
        case 'U':       mask = V_IMMEDIATE;
        case 'u':
                switch(fmt[2]) {
                case 'L':
                case 'l':       *type = V_UL | mask;    return fmt + 3;
                case 'S':
                case 's':       *type = V_US | mask;    return fmt + 3;
                case 'C':
                case 'c':       *type = V_UC | mask;    return fmt + 3;
                case '%':
                case '\0':      *type = V_U | mask;     return fmt + 2;
                default:
                        v_fatal("bogus fmt: expecting [lsc%%eos], got `%c'\n", fmt[2]);
                }
                break;
        case 'P':       mask = V_IMMEDIATE;
        case 'p':       *type = V_P | mask;     return fmt + 2;

        case 'C':       mask = V_IMMEDIATE;
        case 'c':       *type = V_C | mask;     return fmt + 2;

        case 'S':       mask = V_IMMEDIATE;
        case 's':       *type = V_S | mask;     return fmt + 2;

        case 'I':       mask = V_IMMEDIATE;
        case 'i':       *type = V_I | mask;     return fmt + 2;

        case 'L':       mask = V_IMMEDIATE;
        case 'l':       *type = V_L | mask;     return fmt + 2;

        case 'F':       mask = V_IMMEDIATE;
        case 'f':       *type = V_F | mask;     return fmt + 2;

        case 'D':       mask = V_IMMEDIATE;
        case 'd':       *type = V_D | mask;     return fmt + 2;

        case 'b':       *type = V_B;    return fmt + 2;
        default:
                        v_fatal("bogus fmt: expecting [upilfdb], got `%c'\n", fmt[1]);
        }
        /*NOTREACHED*/
        return 0;
}

/* go between string and internal type representation.  */
int *v_xlatel(char *fmt, int *nargs) {
        static int      typei[V_MAXARGS];
        int *ti, i;

        i = 0;
        ti = &typei[0];
        if(fmt) {
                for(; *fmt; ti++, i++)
                        fmt = xlatee(fmt, ti);
        }
        *ti++ = V_ERR;  /* mark end */
        *nargs = i;
        return &typei[0];
}
