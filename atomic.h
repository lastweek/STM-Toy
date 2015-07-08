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
typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;

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
		: "=m" (*p)
		: "r" (mask), "m" (*p)
	);
}

/*
 * Atomic Compare and Exchange.
 *
 * RETURN: the initial value in @MEM.
 * Compare @OLD with @MEM, if identical, store @NEW in memory.
 * Success is indicated by comparing @RETURN with @OLD
 */
#define lock "lock; "
#define atomic_cmpxchg(ptr, old, new)  __cmpxchg(ptr, old, new, sizeof(*(ptr)))
#define __cmpxchg(ptr, old, new, size)					\
({														\
	__typeof__(*(ptr)) __ret;							\
	__typeof__(*(ptr)) __old = (old);					\
	__typeof__(*(ptr)) __new = (new);					\
	switch (size) {										\
	case 1:												\
	{													\
		volatile u8 *__ptr = (volatile u8 *)(ptr);		\
		asm volatile(lock "cmpxchgb %2,%1"				\
			     : "=a" (__ret), "+m" (*__ptr)			\
			     : "q" (__new), "0" (__old)				\
			     : "memory");							\
		break;											\
	}													\
	case 2:												\
	{													\
		volatile u16 *__ptr = (volatile u16 *)(ptr);	\
		asm volatile(lock "cmpxchgw %2,%1"				\
			     : "=a" (__ret), "+m" (*__ptr)			\
			     : "r" (__new), "0" (__old)				\
			     : "memory");							\
		break;											\
	}													\
	case 4:												\
	{													\
		volatile u32 *__ptr = (volatile u32 *)(ptr);	\
		asm volatile(lock "cmpxchgl %2,%1"				\
			     : "=a" (__ret), "+m" (*__ptr)			\
			     : "r" (__new), "0" (__old)				\
			     : "memory");							\
		break;											\
	}													\
	case 8:												\
	{													\
		volatile u64 *__ptr = (volatile u64 *)(ptr);	\
		asm volatile(lock "cmpxchgq %2,%1"				\
			     : "=a" (__ret), "+m" (*__ptr)			\
			     : "r" (__new), "0" (__old)				\
			     : "memory");							\
		break;											\
	}													\
	}													\
	__ret;												\
})

