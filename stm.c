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

struct transaction trans;	// Assume this is the per-thread transaction
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

static int is_committed(struct transaction *t)
{
	return tm_read_status(t) == TM_COMMITED;
}


/**
 * add_after_head - add @new after @ws->head
 * @ws:  write data set
 * @new: new entry added into data set
 */
static void add_after_head(struct write_set *ws, struct w_entry *new)
{
	struct w_entry *tmp;
	
	if (ws->head == NULL) {
		ws->head = new;
		new->next = NULL;
		return;
	}
	
	if (ws->head->next == NULL) {
		ws->head->next = new;
		new->next = NULL;
		return;
	}

	tmp = ws->head->next;
	ws->head->next = new;
	new->next = tmp;
}

/*
 * hash_addr_to_orec
 * @addr: the addr used to hash
 * return: hashed orec in hashtable
 *
 * STM system should keep a hashtable to maintain all
 * ownership records used by an active transaction.
 * After a transaction commit, hashtable should free
 * all transaction relevent records.
 */
//FIXME
static struct orec *hash_addr_to_orec(void *addr)
{
	return NULL;
}

#define DELAY_LOOPS	50
void tm_wait(void)
{
	int i;
	for (i = 0; i < DELAY_LOOPS; i++) {
		asm ("nop");
	}
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
	int status;
	struct w_entry *we, *clean;
	struct orec *rec;
	
	status = tm_read_status(&trans);
	if (status == TM_ABORT)
		return 1;
	
	if (status == TM_ACTIVE) {
		/* Now it is time to write back the dirty data
		 * that have been modified by this transaction. */
		tm_set_status(&trans, TM_COMMITING);
		for (we = trans.ws.head; we != NULL; ) {
			rec = we->rec;
			*(char *)we->addr = rec->new;
			clean = we; /* clean up */
			we = we->next;
			free(clean);
		}
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

void tm_contention_manager(struct transaction *t)
{
	switch (DEFAULT_CM_POLICY) {
		case CM_AGGRESSIVE:
			if (tm_read_status(t) == TM_COMMITING) {
				/* Conflicting transaction is commiting. In order to
				   avoid inconsistency state, abort is really safe */
				tm_abort();
			}
			tm_set_status(t, TM_ABORT);
			break;
		case CM_POLITE:
			/* Backoff to be gentleman */
			/* Future */
			tm_wait();
			break;
	};
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
	struct w_entry *we;
	
	/* Hash addr to get its ownership record */
	rec = hash_addr_to_orec(addr);

	/* FIXME: use oa for test*/
	rec = &oa;
	
	if ((rec->owner != NULL) && (rec->owner != &trans))
		tm_contention_manager(rec->owner);
	
	if (rec->owner == &trans) {
		/* Maybe write data set already has a entry,
		   but that is ok, cause write entry dont have
		   any new data, ownership record has! */
		return rec->new;
	}

	/* Update orec */
	/* Note: Every threads can modify orec */
	data = *(char *)addr;
	orec_set_owner(rec, &trans);
	orec_set_old(rec, data);
	orec_set_new(rec, data);
	
	/* Update transaction write data set */
	/* Note: Transaction belongs to a single thread */
	we = (struct w_entry *)malloc(sizeof(struct w_entry));
	we->addr = addr;
	we->rec  = rec;
	add_after_head(&(rec->owner->ws), we);
	rec->owner->ws.nr_entries += 1;
	
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
	struct w_entry *we;

	/* Hash addr to get its ownership record */
	rec = hash_addr_to_orec(addr);

	/* FIXME: use oa for test*/
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
		
		/* Update transaction write data set */
		/* Note: Transaction belongs to a single thread */
		we = (struct w_entry *)malloc(sizeof(struct w_entry));
		we->addr = addr;
		we->rec  = rec;
		add_after_head(&(rec->owner->ws), we);
		rec->owner->ws.nr_entries += 1;
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
		/* set end time maybe? */		\
	}

#define TM_ABORT()		tm_abort()
#define TM_COMMIT()		tm_commit()
#define TM_VALIDATE()	tm_validate()

#define TM_READ_ADDR(a)		tm_read_addr(a)
#define TM_WRITE_ADDR(a,v)	tm_write_addr(a,v)

#define TM_READ(TMOBJECT)							\
	({												\
		int __i; char __c; char *__addr;			\
		typeof(TMOBJECT)  __ret;					\
		typeof(TMOBJECT) *__retp;					\
		__addr = (char *)&(TMOBJECT);				\
		__retp = &(__ret);							\
		for (__i = 0; __i < sizeof(TMOBJECT); ) {	\
			__c = TM_READ_ADDR(__addr);				\
			*__retp = __c;							\
			__addr++; __retp++; __i++;				\
		}											\
		__ret;										\
	})

#define TM_WRITE(TMOBJECT, VALUE)					\
	{												\
		int __i; char __c; char *__addr;			\
		typeof(TMOBJECT) __v = (VALUE);				\
		typeof(TMOBJECT) *__vp = &__v;				\
		for (__i = 0; __i < sizeof(TMOBJECT); ) {	\
			__c = *((char *)__vp);					\
			TM_WRITE_ADDR(__addr, __c);				\
			__addr++; __vp++; __i++;				\
		}											\
	}

int main(void)
{
	char share = 'A';
	char c;
	int shint = 0x1234;
	int t;

	__TM_START__ {
		
		t = TM_READ(shint);
		
		TM_WRITE(shint, 0x5678);

	} __TM_END__
	
	printf("%d\n", t);
	return 0;
}
