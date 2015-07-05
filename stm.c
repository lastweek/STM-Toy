#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

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

/* Transaction Descriptor */
struct transaction {
	volatile int status;
	jmp_buf jmpbuffer;
	int start;
	int end;
};

static inline void tm_set_status(struct transaction *t, int status)
{
	t->status = status;
}

static inline int tm_read_status(struct transaction *t)
{
	return t->status;
}

void tm_start(struct transaction *t)
{
	memset(t, 0, sizeof(struct transaction));
	tm_set_status(t, TM_ACTIVE);
}

int tm_commit(void)
{
	
}

int tm_validate(void)
{
	int status = tm_read_status(&__t);
	return (status == TM_ACTIVE);
}

void tm_abort(void)
{
	tm_set_status(__t.status, TM_ABORT);
}

#define __TM_START__	\
	tm_start(&__t); setjmp(__t.jmpbuffer);

#define TM_ABORT()		tm_abort()
#define TM_COMMIT()		tm_commit()
#define TM_VALIDATE()	tm_validate()

struct transaction __t;

int main(void)
{
	__TM_START__ {
		printf("%d\n", __t.status);
	}

	return 0;
}




