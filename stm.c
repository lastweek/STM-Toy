#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stm.h"

/* 
 * NOTE THAT: 
 * Each thread only have one transaction descriptor instance.
 * All transactions in a thread share a global descriptpor.
 * I choose to design like that for simplicity, which leads to
 * transactions can not nest.
 *
 * And for now i do not have a hashtable to hash addt to orec.
 * Only a global struct orec oa is used to test!
 */
// FIXME
struct transaction trans;
struct orec	oa;

/* FIXME Multithreads scalability! */
static void tm_set_status(struct transaction *t, int status)
{
	t->status = status;
}

/* FIXME Multithreads scalability! */
static int tm_read_status(struct transaction *t)
{
	return t->status;
}

/* FIXME Multithreads scalability! */
static void orec_set_owner(struct orec *r, struct transaction *t)
{
	r->owner = t;
}

/* FIXME Multithreads scalability! */
static void orec_set_old(struct orec *r, char old)
{
	r->old = old;
}

/* FIXME Multithreads scalability! */
static void orec_set_new(struct orec *r, char new)
{
	r->new = new;
}

static void tm_contention_manager(struct transaction *t)
{
	switch (DEFAULT_CM_POLICY) {
		case CM_AGGRESSIVE:
			tm_set_status(t, TM_ABORT);
			break;
		case CM_POLITE:
			/* Backoff to be gentleman */
			/* Future */
			break;
	};
}

static int is_committed(struct transaction *t)
{
	return tm_read_status(t) == TM_COMMITED;
}


void tm_start(struct transaction *t)
{
	memset(t, 0, sizeof(struct transaction));
	tm_set_status(t, TM_ACTIVE);
}

void tm_abort(void)
{
	tm_set_status(&trans, TM_ABORT);
}

/**
 * tm_commit - Try to commit a transaction
 * Return: 0 means success, 1 means fails.
 */
int tm_commit(void)
{
	int status, d;
	
	status = tm_read_status(&trans);
	if (status == TM_ABORT)
		return 1;
	
	/* Data write back */
	if (status == TM_ACTIVE) {
		tm_set_status(&trans, TM_COMMITED);
	}
	
	return 0;
}

/**
 * tm_validate - Check transaction status
 * Return: 0 means active, 1 means abort
 */
int tm_validate(void)
{
	return tm_read_status(&trans) == TM_ACTIVE;
}

/**
 * tm_read_addr - Read a byte from TM
 * @addr:	Address of the byte
 * Return:	byte dereferenced by @addr
 */
char tm_read_addr(void *addr)
{
	char data;
	struct orec *rec;
	
	/* DEBUG: Hash addr to get orec! */ 

	/* DEBUG: Suppose we got orec already! */
	rec = &oa;
	
	if ((rec->owner != NULL) && (rec->owner != &trans))
		tm_contention_manager(rec->owner);
	
	if (rec->owner == &trans)
		return rec->new;

	data = *(char *)addr;
	orec_set_owner(rec, &trans);
	orec_set_old(rec, data);
	orec_set_new(rec, data);
	
	return data;
}

/**
 * tm_write_addr - Write a byte to TM
 * @addr:	Address of the byte
 */
void tm_write_addr(void *addr, char new)
{
	char old;
	struct orec *rec;

	/* DEBUG: Hash addr to get orec! */

	/* DEBUG: Suppose we got orec too .. */
	rec = &oa;
	
	if ((rec->owner != NULL) && (rec->owner != &trans))
		tm_contention_manager(rec->owner);
	
	if (rec->owner == &trans)
		orec_set_new(rec, new);
	else {
		/* Update orec */
		/* Note: Every threads can modify orec */
		old = *(char *)addr;
		orec_set_owner(rec, &trans);
		orec_set_old(rec, old);
		orec_set_new(rec, new);
		
		/* Update transaction data set */
		/* Note: Transaction belongs to a single thread */

		
	}
}

/*
 *	STM API
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
		/* set end time */				\
	}

#define TM_ABORT()		tm_abort()
#define TM_COMMIT()		tm_commit()
#define TM_VALIDATE()	tm_validate()

#define TM_READ_ADDR(a)		tm_read_addr(a)
#define TM_WRITE_ADDR(a,v)	tm_write_addr(a,v)

int main(void)
{
	char share = 'A';
	char c;
	int t=1;

	TM_START {
		
		c = TM_READ_ADDR(&share);
		printf("%c\n", c);
		
		TM_WRITE_ADDR(&share, 'B');
		c = TM_READ_ADDR(&share);
		printf("%c\n", c);
		
		TM_COMMIT();

	} TM_END

	return 0;
}
