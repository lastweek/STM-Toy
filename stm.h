#ifndef _SYZ_STM_H_
#define _SYZ_STM_H_

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
 * conflict. The waiting time is exponential growth.
 */
enum {
	CM_AGGRESSIVE,
	CM_POLITE
};

#define DEFAULT_CM_POLICY	CM_AGGRESSIVE

/*
 * Transaction Status
 * A transaction is ACTIVE once it starts.
 * A transaction is COMMITED when TM_COMMIT() succeed.
 * A transaction is ABORT after
 *  i)  call TM_ABORT itself
 *  ii) conflict with other transaction
 */
enum TM_STATUS {
	TM_NULL		= 0,
	TM_ACTIVE	= 1,
	TM_COMMITED	= 2,
	TM_ABORT	= 3
};

/*
 * Read Data Entry
 * A single byte that have been read during a
 * transaction has read data entry.
 */
struct r_entry {
	char *addr;
	struct orec *rec;
};

/*
 * Write Data Entry
 * A single byte that have been written to during a
 * transaction has a write data entry.
 */
struct w_entry {
	char *addr;
	struct orec *rec;
};

/*
 * Read Data Set
 * All the data have been read during a transaction
 * form the read data set of that transaction.
 */
struct read_set {
	struct r_entry *rlist;
	int nr_entries;
};

/*
 * Write Data Set
 * All the data have been written to during a transaction
 * form the write data set of that transaction.
 */
struct write_set {
	struct w_entry *wlist;
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
	int start_time;
	int end_time;
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

#endif /* _SYZ_STM_H_ */
