/* $Id: macros-prt.h,v 1.2 1998/05/08 21:48:41 maxp Exp $ */

#ifndef __MACROS_PRT_H__
#define __MACROS_PRT_H__

/*
 * Macros for unparsing icode instructions
 */

#define f_dbop(cp, name) do {					\
     fprintf(fd, name "(r%d, r%d, r%d)\n",				\
	    get_rd(cp), get_rs(cp), get_rs2(cp));		\
} while (0)
#define f_dbopi(cp, name) do {					\
     fprintf(fd, name "(r%d, r%d, %d)\n",				\
	    get_rd(cp), get_rs(cp), get_imm(cp));		\
} while (0)

#define f_dmopr(cp, name) f_dbop(cp, name)
#define f_dmopri(cp, name) f_dbopi(cp, name)
#define f_dmopw(cp, name) f_dbop(cp, name)
#define f_dmopwi(cp, name) f_dbopi(cp, name)

#define f_dset(cp, name) do {					\
     fprintf(fd, name "(r%d, %d)\n", get_rd(cp), 			\
	    get_imm(cp));					\
} while (0)
#define f_duop(cp, name) do {					\
     fprintf(fd, name "(r%d, r%d)\n", get_rd(cp), get_rs(cp));	\
} while (0)
#define f_duopi(cp, name) f_dset(cp, name)

#define f_dret(cp, name) do {					\
     fprintf(fd, name "(r%d)\n", get_rd(cp));			\
} while (0)
#define f_dreti(cp, name) do {					\
     fprintf(fd, name "(%p)\n", (void*)get_imm(cp));			\
} while (0)

#define f_djmp(cp, name) f_dret(cp, name)
#define f_djmpi(cp, name) f_dreti(cp, name)

#define f_dbr(cp, name) do {					\
     fprintf(fd, name "(r%d, r%d, L%d)\n",				\
	    get_rs(cp), get_rs2(cp), get_rd(cp));		\
} while (0)
#define f_dbri(cp, name) do {					\
     fprintf(fd, name "(r%d, %d, L%d)\n",				\
	    get_rs(cp), get_imm(cp), get_rd(cp));		\
} while (0)

#define f_dcall(cp, name) do {					\
     fprintf(fd, name "(r%d, r%d)\n", get_rd(cp), get_rs(cp));	\
} while (0)
#define f_dcalli(cp, name) do {					\
     fprintf(fd, name "(r%d, %p)\n", get_rd(cp), 			\
	(void*)get_imm(cp));					\
} while (0)

#define f_darg(cp, name) do {					\
     fprintf(fd, name "(r%d)\n", get_rd(cp));			\
} while (0)
#define f_dlea(cp, name) f_duop(cp, name)

#endif
