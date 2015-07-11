/*
 * Thread-Local Storage
 *
 * There are many ways to define and use TLS
 * variables. The default way is using GCC
 * supported keyword: __thread. Maybe we can
 * add support for pthread specific function
 * set in the future. ;)
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
