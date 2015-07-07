
struct tm_tx *get_tx(void)
{
	return tx;
}

void set_tx(struct stm_tx *new_tx)
{
	tx = new_tx;
}
