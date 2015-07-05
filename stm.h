#ifndef _SYZ_STM_H_
#define _SYZ_STM_H_

/* Contention Policy */
enum {
	CM_AGGRESIVE,
	CM_POLITE
};
#define CM_POLICY	CM_AGGRESIVE

/* Transaction Status */
enum TM_STATUS {
	TM_NULL		= 0,
	TM_ACTIVE	= 1,
	TM_COMMITED	= 2,
	TM_ABORT	= 3
};

/* Read Data Entry in Read-Set */
struct r_entry {

};

/* Write Data Entry in Write-Set */
struct w_entry {

};

/* Read-Set in Transaction */
struct read_set {
	struct r_entry *rlist;
	int nr_entries;
};

/* Write-Set in Transaction */
struct write_set {
	struct w_entry *wlist;
	int nr_entries;
};

/* Transaction Descriptor */
struct transaction {
	jmp_buf jb;
	volatile int status;
	struct read_set  rs;
	struct write_set ws;
	int start_time;
	int end_time;
};

/* Ownership RECord for every TM byte */
struct orec {
	struct transaction *owner;
	int version;
	char old;
	char new;
};

#endif /* _SYZ_STM_H_ */
