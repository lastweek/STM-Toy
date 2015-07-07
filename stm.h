#ifndef _SYZ_STM_H_
#define _SYZ_STM_H_

#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TM_DEBUG
#define TM_STATISTICS

/*
 *	STM API
 *
 *	Note that with this STM implementation, you do not
 *	need to specify any memory region as Transactional Memory.
 *	When you access the shared object using STM API in transaction,
 *	for example, TM_READ() or TM_WRITE(), the system will mark the
 *	memory region that the shared object cover as TM automatically.
 *	And, yes, the granularity is one single byte, each byte in TM
 *	has a ownership record depict itself.
 */
#define __TM_START__					\
	{									\
		tm_start(&trans);				\
		setjmp(trans.jb);				\
	}

#define __TM_END__						\
	{									\
		if (!is_committed(&trans)) {	\
			longjmp(trans.jb, 1);		\
		}								\
		/* set end time maybe? */		\
		/* add statistics maybe? */		\
	}

#define TM_ABORT()		tm_abort()
#define TM_COMMIT()		tm_commit()
#define TM_VALIDATE()	tm_validate()
#define TM_BARRIER()	tm_barrier()

#define TM_READ_ADDR(a)		tm_read_addr(a)
#define TM_WRITE_ADDR(a,v)	tm_write_addr(a,v)

#define TM_READ(TMOBJECT)								\
	({													\
		int __i; char __c;								\
		typeof(TMOBJECT)  __ret;						\
		char *__retp = (char *)&(__ret);				\
		char *__addr = (char *)&(TMOBJECT);				\
		for (__i = 0; __i < sizeof(TMOBJECT); ) {		\
			__c = TM_READ_ADDR(__addr);					\
			*__retp = __c;								\
			__addr++; __retp++; __i++;					\
		}												\
		__ret;											\
	})

#define TM_WRITE(TMOBJECT, VALUE)						\
	{													\
		int __i; char __c;								\
		typeof(TMOBJECT) __val = (VALUE);				\
		char *__valp = (char *)&(__val);				\
		char *__addr = (char *)&(TMOBJECT);				\
		for (__i = 0; __i < sizeof(TMOBJECT); ) {		\
			__c = *((char *)__valp);					\
			TM_WRITE_ADDR(__addr, __c);					\
			__addr++; __valp++; __i++;					\
		}												\
	}

/*
 * Unlike SigSTM, we can NOT guarantee strong isolation
 * between transactional and non-transactional code.
 * tm_barrier() will saddle non-transactional code with
 * additional overheads. Because processor can NOT classify
 * where the memory access come from, TM or non-TM. So, all
 * memory access can NOT across barrier no matter who you are.
 */
static inline void tm_barrier(void)
{

}



/*
 * Contention Policy
 * 
 * AGGRESSIVE is an obstruction-free implementation.
 * The transaction just abort all other transactions
 * that holds the orec of the data that this transaction
 * want to access.
 *
 * POLITE is an more gentle implementation.
 * The transaction will wait a moment when encounter a
 * conflict. The waiting time has a exponential growth.
 */
enum {
	CM_AGGRESSIVE,
	CM_POLITE
};

#define DEFAULT_CM_POLICY	CM_AGGRESSIVE

/*
 * Transaction Status
 * A transaction is ACTIVE once it starts, until ABORT or COMMIT
 * A transaction is COMMITING when TM_COMMIT() is flushing dirty data.
 * A transaction is COMMITED when TM_COMMIT() has finished and succeed.
 * A transaction is ABORT when two actions happened:
 *  i)  call TM_ABORT() itself
 *  ii) ABORT by the conflicting transaction
 */
enum TM_STATUS {
	TM_NULL,
	TM_ACTIVE,
	TM_COMMITING,
	TM_COMMITED,
	TM_ABORT
};

/*
 * Read Data Entry
 * A single byte that have been read during a
 * transaction has read data entry.
 */
struct r_entry {
	char *addr;
	struct orec *rec;
	struct r_entry *next;
};

/*
 * Write Data Entry
 * A single byte that have been written to during a
 * transaction has a write data entry.
 */
struct w_entry {
	void *addr;
	struct orec *rec;
	struct w_entry *next;
};

/*
 * Read Data Set
 * All the data have been read during a transaction
 * form the read data set of that transaction.
 */
struct read_set {
	struct r_entry *head;
	int nr_entries;
};

/*
 * Write Data Set
 * All the data have been written to during a transaction
 * form the write data set of that transaction.
 */
struct write_set {
	struct w_entry *head;
	int nr_entries;
};

/*
 * Transaction Descriptor
 * Each transaction have a descriptor depicts
 * its status, data sets and so on.
 */
struct transaction {
	jmp_buf jb;
	volatile int status;
	struct read_set  rs;
	struct write_set ws;
	int start_tsp;	/* Start TimeStamp */
	int end_tsp;	/* End TimeStamp */
#ifdef TM_STATISTICS
	int nr_aborts;
#endif
};

/*
 * Ownership Record
 * Every byte in TM has its own record. The size of
 * the record is 8+4+4=16 bytes in 64-bit system.
 * Therefore, if you have 1M transactional memory,
 * the total records will consume 16M.
 */
struct orec {
	struct transaction *owner;
	int version;
	char old;
	char new;
	char pad[2];
};

void tm_wait(void);
void tm_start(struct transaction *t);
void tm_abort(void);
int tm_commit(void);
int tm_validate(void);
char tm_read_addr(void *addr);
void tm_write_addr(void *addr, char new);
void tm_contention_manager(struct transaction *t);

#endif /* _SYZ_STM_H_ */
