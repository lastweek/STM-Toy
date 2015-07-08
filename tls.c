/*
 * Thread-Local Storage
 */

#include "stm.h"

struct stm_tx *tls_get_tx(void)
{
	return thread_tx;
}

void tls_set_tx(struct stm_tx *new_tx)
{
	thread_tx = new_tx;
}
