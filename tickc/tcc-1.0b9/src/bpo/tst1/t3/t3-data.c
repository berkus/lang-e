#include "bpo.h"

iv_t r0i0v[] = {
	{ 0x0, 0, 0x7},
	{ 0 }
};
iv_t r0i1v[] = {
	{ 0x1, 0, 0x7},
	{ 0 }
};
iw_t r0i[] = {
	{ 0xf8, 0x58, 1, r0i0v },
	{ 0xf8, 0x58, 1, r0i1v },
	{ 0 }
};
ov_t r0o0v[] = {
	{ 0x0, 0 },
	{ 0 }
};
ov_t r0o2v[] = {
	{ 0x1, 0 },
	{ 0 }
};
ow_t r0o[] = {
	{ 0x58, 1, r0o0v },
	{ 0x90, 0, 0 },
	{ 0x58, 1, r0o2v },
	{ 0x90, 0, 0 },
	{ 0 }
};
int r0rd[] = {
	-1,
	-1,
	-1,
	-1,
	0
};
rule_t bpoR[] = {
	{ 2, r0i, 4, r0o, r0rd },
	{ 0 }
};
int bpoNR = 1;
