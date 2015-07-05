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
