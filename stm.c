#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "stm.h"

/* FIXME Tentative Global...
 * For now, assume that one thread only
 * have one struct transaction instance.
 * And should use hashtable to index orec. */
struct transaction trans;
struct orec	oa, ob;

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
};

void tm_start(struct transaction *t)
{
	memset(t, 0, sizeof(struct transaction));
	tm_set_status(t, TM_ACTIVE);

void tm_abort(void)
{
	tm_set_status(&trans, TM_ABORT);
}
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

/**
 * tm_read_addr - Read a byte from TM
 * @addr:	Address of the byte
 * Return:	byte dereferenced by @addr
 */
char tm_read_addr(void *addr)
{
	struct orec *rec;
	
	/* Hash addr to get orec! */ 

	/* DEBUG: Suppose we got orec already! */
	rec = &oa;
	
	if (rec.owner != NULL)
		tm_contention_manager(rec.owner);
	
	orec_set_owner(rec, &trans);

	return *(char *)addr;
}

/**
 * tm_write_addr - Write a byte to TM
 * @addr:	Address of the byte
 */
void tm_write_addr(void *addr)
{

}

void tm_contention_manager(struct transaction *t)
{
	switch (CM_POLICY) {
		case CM_AGGRESIVE:
			/* Make conflict transaction abort */
			
			break;
		case CM_POLITE:
			/* Backoff to be gentleman */
			
			break
		default:
	};
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




