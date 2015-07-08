
struct stm_tx *tls_get_tx(void)
{
	return tx;
}

void tls_set_tx(struct stm_tx *new_tx)
{
	tx = new_tx;
}
