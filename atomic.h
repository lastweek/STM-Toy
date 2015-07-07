/*
 * Atomic operations, once started, the processor guarantees
 * that the operation will be completed before another processor
 * or bus agent is allowed access to the memory location.
 *
 * The processor also supports bus LOCKing for performing selected
 * memory operations (such as a read-modify-write operation in a
 * shared area of memory) that typically need to be handled atomically,
 * but are not automatically handled this way.
 */

#define LOCK	"lock ; "

static inline char
atomic_read_char(int *addr)
{
	return *(char *)addr;
}

static inline int
atomic_read_int(int *addr)
{
	return *addr;
}

static inline void
atomic_write_char(int *addr, char val)
{
	*((char *)addr) = val;
}

static inline void
atomic_write_int(int *addr, int val)
{
	*addr = val;
}

static inline void
atomic_or(int *addr, int mask)
{
	asm volatile (
		LOCK "orl %1, %0"
		: "=m" (*addr)
		: "r" (mask), "0" (*addr)
	);
}

static inline int
compare_and_set()
{

}
