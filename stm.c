#include <stdio.h>
#include <string.h>
#include <setjmp.h>

/* FIXME Tentative Global...
 * For now, assume that one thread only
 * have one struct transaction instance */
struct transaction trans;

/* Contention Policy */
enum CM_POLICY {
	CM_AGGRESIVE,
	CM_POLITE
};

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

/* Ownership record for every TM byte */
struct ownership_record {
	struct transaction *owner;
	int version;
	char old;
	char new;
};

static void tm_set_status(struct transaction *t, int status)
{
	t->status = status;
}

static int tm_read_status(struct transaction *t)
{
	return t->status;
}

/**
 * tm_start - TM start and initialize
 * @t:	transaction descriptor
 */
void tm_start(struct transaction *t)
{
	memset(t, 0, sizeof(struct transaction));
	tm_set_status(t, TM_ACTIVE);
}

/**
 * tm_commit - Try to commit a transaction
 * Return: 0 means success, 1 means fails.
 */
int tm_commit(void)
{
	int status;
	status = tm_read_status(&trans);
	
	tm_set_status(&trans, TM_COMMITED);
	
	return (status == TM_ABORT);
}

/**
 * tm_validate - Check transaction status
 * Return: 0 means active, 1 means abort
 */
int tm_validate(void)
{
	int status;
	status = tm_read_status(&trans);
	return (status == TM_ACTIVE);
}

void tm_abort(void)
{
	tm_set_status(&trans, TM_ABORT);
}

/**
 * tm_read_addr - Read a byte from TM
 * @addr:	Address of the byte
 * Return:	byte dereferenced by @addr
 */
char tm_read_addr(void *addr)
{
	/* Map to orec */


	return *(char *)addr;
}

/**
 * tm_write_addr - Write a byte to TM
 * @addr:	Address of the byte
 */
void tm_write_addr(void *addr)
{

}



/* General Purpose STM Programming API */
#define __TM_START__				\
	{								\
		tm_start(&trans);			\
		setjmp(trans.jb);			\
	}

#define __TM_END__					\
	{								\
		if (tm_commit()) {			\
			/* Start again */		\
			longjmp(trans.jb, 1);	\
		}							\
		/* set end time */			\
	}

#define TM_ABORT()		tm_abort()
#define TM_COMMIT()		tm_commit()
#define TM_VALIDATE()	tm_validate()

#define TM_READ_ADDR(p)		tm_read_addr(p)
#define TM_WRITE_ADDR(p)	tm_write_addr(p)

int main(void)
{
	char share = 'A';
	int t=1;

	__TM_START__ {
		
	
	}__TM_END__

	return 0;
}




