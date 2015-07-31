/*
 *	Thread-Local Storage
 *	There are many ways to define and use TLS variables. The default way is
 *	using GCC supported keyword: __thread. Maybe we can add support for
 *	pthread specific function set in the future. ;)
 *
 *	Copyright (C) 2015 Yizhou Shan
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
