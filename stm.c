#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

/* FIXME Tentative Global... */
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

struct r_entry {

};
struct w_entry {

};

/* Read Data Set in Transaction */
struct r_set {
	struct r_entry *rlist;
	int nr_entries
};

/* Write Data Set in Transaction */
struct w_set {
	struct w_entry *wlist;
	int nr_entries
};

/* Transaction Descriptor */
struct transaction {
	volatile int status;
	jmp_buf jb;
	struct r_set rs;
	struct w_set ws;
	int start_time;
	int end_time;
};

static inline void tm_set_status(struct transaction *t, int status)
{
	t->status = status;
}

static inline int tm_read_status(struct transaction *t)
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
	
}

/**
 * tm_validate - Check transaction status
 * Return: 0 means active, 1 means abort
 */
int tm_validate(void)
{
	int status = tm_read_status(&trans);
	return (status == TM_ACTIVE);
}

void tm_abort(void)
{
	tm_set_status(trans.status, TM_ABORT);
}

/**
 * tm_read_addr - Read a byte from TM
 * @addr:	Address of the byte
 * Return:	byte dereferenced by @addr
 */
char tm_read_addr(void *addr)
{

}

/**
 * tm_write_addr - Write a byte to TM
 * @addr:	Address of the byte
 */
void tm_write_addr(void *addr)
{

}

/* General Purpose STM Programming API */
#define __TM_START__	\
	tm_start(&trans); setjmp(trans.jb);

#define TM_ABORT()		tm_abort()
#define TM_COMMIT()		tm_commit()
#define TM_VALIDATE()	tm_validate()

#define TM_READ_ADDR(p)		tm_read_addr(p)
#define TM_WRITE_ADDR(p)	tm_write_addr(p)

int main(void)
{
	__TM_START__ {
		printf("%d\n", __t.status);
	}

	return 0;
}




