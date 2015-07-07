/*
 * From Intel SDM:
 * Atomic operations, once started, the processor guarantees
 * that the operation will be completed before another processor
 * or bus agent is allowed access to the memory location.
 *
 * The processor also supports bus LOCKing for performing selected
 * memory operations (such as a read-modify-write operation in a
 * shared area of memory) that typically need to be handled atomically,
 * but are not automatically handled this way.
 */

typedef int atomic_t;

static inline char atomic_read_char(atomic_t *p)
{
	return *(char *)p;
}

static inline void atomic_write_char(atomic_t *p, char val)
{
	*((char *)p) = val;
}

static inline int atomic_read(atomic_t *p)
{
	return *p;
}

static inline void atomic_write(atomic_t *p, int val)
{
	*p = val;
}

static inline void atomic_or(atomic_t *p, int mask)
{
	asm volatile (
		"lock ; orl %1, %0"
		: "=m" (*addr)
		: "r" (mask), "0" (*addr)
	);
}

/*
 * Atomic Compare and Exchange.
 *
 * RETURN: the initial value in @MEM.
 * Compare @OLD with @MEM, if identical, store @NEW in memory.
 * Success is indicated by comparing @RETURN with @OLD
 */
static inline int atomic_cmpxchg(atomic_t *p, int old, int new)
{

}




