#include "atomic.h"
#include "stm.h"
/* 
 * NOTE THAT: 
 * Each thread only have one transaction descriptor instance.
 * All transactions in a thread _share_ a global descriptpor.
 * I choose to design like that for simplicity, which leads to
 * transactions can not nest.
 */


DEF_THREAD_LOCAL(struct stm_tx *, thread_tx);

struct orec	oa[4];
int cnt=0;


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
void stm_wait(void)
{
	int i;
	for (i = 0; i < DELAY_LOOPS; i++) {
		asm ("nop":::"memory");//Compiler Barrier
	}
}


//#################################################
// 
//#################################################

void stm_init(void)
{

}

void stm_thread_init(void)
{
	thread_tx = NULL;
}

/*
 *	Allocate _aligned_ tx descriptor
 *	and zero it up.
 */
struct stm_tx *TX_MALLOC(size_t size)
{
	void *memptr;
	
	//TODO
	memptr = valloc(size);
	
	memset(memptr, 0, size);
	return (struct stm_tx *)memptr;
}

//TODO
int current_tsp(void)
{
	return 0;
}

/*
 *	Allocate memory for thread-local tx descriptor
 *	if it is the first time this thread use transaction.
 *	Initialize the variables in tx descriptor.
 */
void stm_start(void)
{
	struct stm_tx *new_tx;
	
	/* Thread Global TX has initialized */
	if (tls_get_tx())
		return;
	
	new_tx = TX_MALLOC(sizeof(struct stm_tx));
	new_tx->status = STM_ACTIVE;
	new_tx->version = 0;
	new_tx->start_tsp = current_tsp();
	tls_set_tx(new_tx);
}

/*
 * Maybe there are more than one transaction
 * try to abort the same tx in the meantime.
 * That is ok, because abort() only change
 * the status of the tx.
 */
void stm_abort(void)
{
	stm_set_status(STM_ABORT);
	stm_set_abort_reason(STM_SELF_ABORT);
}

void stm_abort_tx(struct stm_tx *tx, int reason)
{
	stm_set_status_tx(tx, STM_ABORT);
	stm_set_abort_reason_tx(tx, STM_ENEMY_ABORT);
}

/**
 * stm_commit - TRY to commit a transaction
 * Return: 0 means success, 1 means fails.
 */
int stm_commit(void)
{
	int status;
	struct w_entry *we, *clean;
	struct orec *rec;
	
	/* COMPARE AND SWAP 
	 * If current status is ACTIVE, then set current
	 * status to COMMITING. A COMMITING transaction
	 * can NOT be aborted by other transactions, in
	 * other words, it is un-interruptable.
	 */
	GET_TX(tx);
	if (atomic_cmpxchg(&tx->status, STM_ACTIVE, STM_COMMITING) == STM_ACTIVE) {
		/* Now it is time to write back the dirty data
		 * that have been modified by this transaction.
		 * We are in COMMITING state, no one can abort us. */
		for (we = tx->ws.head; we != NULL; ) {
			rec = we->rec;
			*(char *)we->addr = rec->new;
			clean = we;
			we = we->next;
			free(clean);
		}
		/* Wait until data flush out */
		stm_barrier();
		stm_set_status_tx(tx, STM_COMMITED);
		return 0;
	}
	return 1;
}

/**
 * stm_validate - check transaction status
 * ACTIVE, COMMITING and COMMITED indicate
 * the transaction still alive.
 * Return: 0 means alive, 1 means aborted
 */
int stm_validate(void)
{
	return stm_get_status() == STM_ABORT;
}

int stm_validate_tx(struct stm_tx *tx)
{
	return stm_get_status_tx(tx) == STM_ABORT;
}

int stm_contention_manager(struct stm_tx *enemy)
{
	GET_TX(tx);
	switch (DEFAULT_CM_POLICY) {
		case CM_AGGRESSIVE:
			/* As the promise we have made, this conflicting
			 * transaction can NOT abort a COMMITING enemy.
			 * TODO Maybe abort himself or wait a moment?
			 */
			if (stm_get_status_tx(enemy) == STM_COMMITING) {
				stm_abort_tx(tx, STM_SELF_ABORT);
				return STM_SELF_ABORT;
			}
			else {
				stm_abort_tx(enemy, STM_ENEMY_ABORT);
				return STM_ENEMY_ABORT;
			}
		case CM_POLITE:
			/* Future */
			stm_wait();
			return STM_SELF_ABORT;
		default:
			return STM_SELF_ABORT;
	};
}

/**
 * stm_read_addr - Read a single BYTE from STM
 * @addr:	Address of the byte
 * Return:	the date in @addr
 */
char stm_read_char(void *addr)
{
	char data;
	struct orec *rec;
	struct w_entry *we;
	struct stm_tx *enemy;
	GET_TX(tx);
	
	if (stm_validate_tx(tx)) {
		//TODO Restart transaction maybe?
		//It is dangerous to return 0, cause application
		//may use return value as a pointer.
		return 0;
	}
	
	//FIXME Get the ownership record
	rec = hash_addr_to_orec(addr);
	rec = &oa[cnt%4]; cnt++;
	
	if (enemy = atomic_cmpxchg(&rec->owner, NULL, tx)) {
		/* Other tx may abort thix tx IN THE MEANTIME.
		 * However, we just return the new data in OREC.
		 * If this tx is aborted by other tx, then
		 * in next read/write or commit time, this
		 * tx will restart, so consistency ensured */
		if (enemy == tx)
			return OREC_GET_NEW(rec);

		/* Otherwise, an enemy has already owns this OREC.
		 * Now let contention manager to decide the coin. */
		if (stm_contention_manager(enemy) == STM_SELF_ABORT) {
			//TODO
			return 0;
		}
	}
	
	/*
	 * When we reach here, it means OREC has NO owner,
	 * or, OREC has a owner who contention manager has
	 * already decided to abort.
	 * BUT, other tx may abort this tx right now...
	 */
	if (enemy == NULL) {
		data = *(char *)addr;
		OREC_SET_OLD(rec, data);
		OREC_SET_NEW(rec, data);
		
		/* Update transaction write data set */
		we = (struct w_entry *)malloc(sizeof(struct w_entry));
		we->addr = addr;
		we->rec  = rec;
		add_after_head(&(rec->owner->ws), we);
		rec->owner->ws.nr_entries += 1;
		return data;
	} else {
		//some tx owned OREC, but aborted now.
		//Reclaim the owner of OREC
		//TODO For simplicity, do nothing.
		return 0;
	}
}

/**
 * tm_write_addr - Write a byte to TM
 * @addr:	Address of the byte
 */
void stm_write_char(void *addr, char new)
{
	char old;
	struct orec *rec;
	struct w_entry *we;
	struct stm_tx *enemy;
	GET_TX(tx);
	
	if (stm_validate_tx(tx)) {
		//TODO Restart transaction maybe?
		//It is dangerous to return 0, cause application
		//may use return value as a pointer.
		return;
	}
	
	//FIXME Get the ownership record
	rec = hash_addr_to_orec(addr);
	rec = &oa[cnt%4]; cnt++;
	
	if (enemy = atomic_cmpxchg(&rec->owner, NULL, tx)) {
		if (enemy == tx)
			return;

		if (stm_contention_manager(enemy) == STM_SELF_ABORT) {
			//TODO
			return;
		}
	}
	
	if (enemy == NULL) {
		old = *(char *)addr;
		OREC_SET_OLD(rec, old);
		OREC_SET_NEW(rec, new);
		
		/* Update transaction write data set */
		we = (struct w_entry *)malloc(sizeof(struct w_entry));
		we->addr = addr;
		we->rec  = rec;
		add_after_head(&(rec->owner->ws), we);
		rec->owner->ws.nr_entries += 1;
		return;
	} else {
		//some tx owned OREC, but aborted now.
		//Reclaim the owner of OREC
		//TODO For simplicity, do nothing.
		return;
	}
}

struct str {
	char a;
	char b;
	char c;
	char d;
};

int main(void)
{
	char share = 'A';
	char c;
	int shint = 0x05060708;
	int s, t;
	struct str x, y;
	struct str z = {.a='A', .b='B', .c='C', .d='D'};
	
	__TM_START__ {
		
		x = TM_READ(z);
		y = TM_READ(z);

		TM_WRITE(shint, 0x01020304);
		TM_WRITE(shint, 0x04030201);
		
		TM_COMMIT();

	} __TM_END__

	printf("%c %c %c %c\n", x.a, x.b, x.c, x.d);
	printf("%x\n", shint);
	return 0;
}
