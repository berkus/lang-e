#include <stdio.h>
#include <stdlib.h>


#if (defined(SPARC) || defined(__sparc__)) && defined(__GNUC__) && !defined(_V_SOLARIS_) && !defined(__SVR4)
extern int printf(char *, ...);
#endif

#define ADD_COST 1
#define SUB_COST 1
#define NEG_COST 1
#define SHIFT_COST 1
#define HASH_SIZE 511

#undef verbose

static int      t, n, src;

typedef
enum {
	IDENTITY,		/* used for n = 1          */
	NEGATE,			/* used for n = -1         */
	SHIFT_ADD,		/* used for makeOdd(n - 1) */
	SHIFT_SUB,		/* used for makeOdd(n + 1) */
	SHIFT_REV,		/* used for makeOdd(1 - n) */
	FACTOR_ADD,		/* used for n/(2^i - 1)    */
	FACTOR_SUB,		/* used for n/(2^i + 1)    */
	FACTOR_REV		/* used for n/(1 - 2^i)    */
}               MulOp;

typedef
struct node {
	struct node    *parent;
	int             value;
	unsigned int    cost;
	MulOp           opcode;
	struct node    *next;
}               Node;

static unsigned int costs[] =
{0,				/* for IDENTITY   */
	NEG_COST,		/* for NEGATE     */
	SHIFT_COST + ADD_COST,	/* for SHIFT_ADD  */
	SHIFT_COST + SUB_COST,	/* for SHIFT_SUB  */
	SHIFT_COST + SUB_COST,	/* for SHIFT_REV  */
	SHIFT_COST + ADD_COST,	/* for FACTOR_ADD */
	SHIFT_COST + SUB_COST,	/* for FACTOR_SUB */
	SHIFT_COST + SUB_COST	/* for FACTOR_REV */
};

static Node    *hash_table[HASH_SIZE];

#define odd(c) ((c) & 1)
#define even(c) (!odd(c))

static int 
makeOdd(int c)
{
	do
		c = c / 2;
	while (even(c));
	return c;
}

static Node    *
lookup(int c)
{
	int             hash = abs(c) % HASH_SIZE;
	Node           *node = hash_table[hash];
	while (node && node->value != c)
		node = node->next;
	if (!node) {
		node = (Node *) malloc(sizeof(Node));
		node->value = c;
		node->parent = NULL;
		node->cost = SHIFT_COST + 1;

		node->next = hash_table[hash];
		hash_table[hash] = node;
	}
	return node;
}

static void 
emit_shift(int target, int source)
{
	int             temp = source;
	unsigned int    i = 0;
	do {
		temp <<= 1;
		i++;
	} while (target != temp);
#ifdef verbose
	fprintf(stdout, "%d = %d << %d\n", target, source, i);
#endif
	n = i;
}

static int 
emit_code(Node * node)
{
	int             source;
#if 0
	unsigned int    shift;
#endif
	int             target = node->value;
	switch (node->opcode) {
	case IDENTITY:
		break;
	case NEGATE:
		source = emit_code(node->parent);
		printf("\tNEGATE,");
		t = -t;
#ifdef verbose
		fprintf(stdout, "%d = 0 - %d\n", target, source);
#endif
		break;
	case SHIFT_ADD:
		source = emit_code(node->parent);
		emit_shift(target - 1, source);
#ifdef verbose
		fprintf(stdout, "%d = %d + 1\n", target, target - 1);
#endif
		printf("\tSHIFT_ADD, %d,", n);
		t = (t << n) + src;
		break;
	case SHIFT_SUB:
		source = emit_code(node->parent);
		emit_shift(target + 1, source);
#ifdef verbose
		fprintf(stdout, "%d = %d - 1\n", target, target + 1);
#endif
		printf("\tSHIFT_SUB, %d,",n);
		t = (t << n) - src;
		break;
	case SHIFT_REV:
		source = emit_code(node->parent);
		emit_shift(1 - target, source);
#ifdef verbose
		fprintf(stdout, "%d = 1 - %d\n", target, 1 - target);
#endif
		printf("\tSHIFT_REV, %d,",n);
		t = src - (t << n);
		break;
	case FACTOR_ADD:
		source = emit_code(node->parent);
		emit_shift(target - source, source);
#ifdef verbose
		fprintf(stdout, "%d = %d + %d\n", target, target - source, source);
#endif
		t = (t << n) + t;
		printf("\tFACTOR_ADD, %d,",n);
		break;
	case FACTOR_SUB:
		source = emit_code(node->parent);
		emit_shift(target + source, source);
#ifdef verbose
		fprintf(stdout, "%d = %d - %d\n", target, target + source, source);
#endif
		t = (t << n) - t;
		printf("\tFACTOR_SUB, %d,",n);
		break;
	case FACTOR_REV:
		source = emit_code(node->parent);
		emit_shift(source - target, source);
#ifdef verbose
		fprintf(stdout, "%d = %d - %d\n", target, source, source - target);
#endif
		t = t - (t << n);
		printf("\tFACTOR_REV, %d,",n);
		break;
	}
	return target;
}

static Node    *find_sequence(int c, unsigned int limit);

static void 
try(int factor, Node * node, MulOp opcode)
{
	unsigned int    cost = costs[opcode];
	unsigned int    limit = node->cost - cost;
	Node           *factor_node = find_sequence(factor, limit);
	if (factor_node->parent && factor_node->cost < limit) {
		node->parent = factor_node;
		node->opcode = opcode;
		node->cost = factor_node->cost + cost;
	}
}

static Node    *
find_sequence(int c, unsigned int limit)
{
	Node           *node = lookup(c);
	if (!node->parent && node->cost < limit) {
		node->cost = limit;
		if (c > 0) {
			int             power = 4;
			int             edge = c >> 1;
			while (power < edge) {
				if (c % (power - 1) == 0)
					try(c / (power - 1), node, FACTOR_SUB);
				if (c % (power + 1) == 0)
					try(c / (power + 1), node, FACTOR_ADD);
				power = power << 1;
			}
			try(makeOdd(c - 1), node, SHIFT_ADD);
			try(makeOdd(c + 1), node, SHIFT_SUB);
		} else {
			int             power = 4;
			int             edge = (-c) >> 1;
			while (power < edge) {
				if (c % (1 - power) == 0)
					try(c / (1 - power), node, FACTOR_REV);
				if (c % (power + 1) == 0)
					try(c / (power + 1), node, FACTOR_ADD);
				power = power << 1;
			}
			try(makeOdd(1 - c), node, SHIFT_REV);
			try(makeOdd(c + 1), node, SHIFT_SUB);
		}
	}
	return node;
}

static unsigned int 
estimate_cost(int c)
{
	/*
	 * if (c >= 0) if (c >= 65536) return 9; else return 5; else if (c <=
	 * -65536) return 9; else return 5;
	 */
	return 20;
}

void 
multiply(int target)
{
	unsigned int    multiply_cost = estimate_cost(target), cost;
	if (odd(target)) {
		Node           *result = find_sequence(target, multiply_cost);
		if (result->parent && (cost = result->cost) < multiply_cost) {
			printf("\t%d,",cost);
			(void) emit_code(result);
		}
#ifdef verbose
		else
		     fprintf(stdout, "use multiply instruction\n");
#endif
	} else {
		Node           *result = find_sequence(makeOdd(target), multiply_cost - SHIFT_COST);
		if (result->parent && (cost = (result->cost + SHIFT_COST)) < multiply_cost) {
			int             source;
			printf("\t%d,",cost);
			source = emit_code(result);
			emit_shift(target, source);
			t = t << n;
			printf("\tSHIFT, %d,",n);
		} 
#ifdef verbose
		else
		     	fprintf(stdout, "use multiply instruction\n");
#endif
	}
}

void 
init_multiply(void)
{
	Node           *node, *node1;
	unsigned int    i;
	for (i = 0; i < HASH_SIZE; i++)
		hash_table[i] = NULL;

	node1 = lookup(1);
	node1->parent = node1;	/* must not be NULL */
	node1->opcode = IDENTITY;
	node1->cost = 0;

	node = lookup(-1);
	node->parent = node1;
	node->opcode = NEGATE;
	node->cost = NEG_COST;
}

void 
main(int argc, char *argv[])
{
	int             i, scaler = atoi(argv[1]);
	init_multiply();

	printf(" enum {    \n"
	"\t	IDENTITY, /* used for n = 1          */			\n"
	"\t	NEGATE,			/* used for n = -1         */   \n"
	"\t	SHIFT_ADD,		/* used for makeOdd(n - 1) */   \n"
	"\t	SHIFT_SUB,		/* used for makeOdd(n + 1) */   \n"
	"\t	SHIFT_REV,		/* used for makeOdd(1 - n) */   \n"
	"\t	FACTOR_ADD,		/* used for n/(2^i - 1)    */   \n"
	"\t	FACTOR_SUB,		/* used for n/(2^i + 1)    */   \n"
	"\t	FACTOR_REV,		/* used for n/(1 - 2^i)    */   \n"
	"\t	SHIFT,							\n"
	"\t	DONE							\n"
	" };\n\n");


	for(i = 1; i <= scaler; i++) {
		printf("static char mul%d[] = {",i);
		multiply(i);
		printf("\tDONE };\t/* %d */\n",i);
	}
	printf("\n#define LSIZE %d\n",scaler+1);
	printf("\n\nstatic char *mlookup[LSIZE] = {\n\t0,\n");
	for(i = 1; i <= scaler; i++) {
		printf("\tmul%d,\n",i);			
	}
	printf("};\n");

#if 0
	while (EOF != scanf("%d", &i)) {
		src = t = 2;
		if (i)
			multiply(i);
		printf("%d * %d = %d,  t = %d\n", i, 2, i * 2, t);
	}
#endif

	exit(0);
}
