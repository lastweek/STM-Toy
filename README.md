#Software Transactional Memory

This is an immature prototype of STM system.
I borrowed some ideas from DSTM and Hash Table STM.

Like Hash Table STM, we hash the address of
the shared object to get the ownership record
of that shared object. Like DSTM, we store three
parameters in ownership record: pointer to the
transaction who owns this object, the old verison
data, the new version data.

When a transaction use TM-READ or TM-WRITE to access
the shared object, the underlying system will mark
address region that shared object cover as TM automatically.
Once the address region become TM, it will have a
ownership record depict its status and data.

The default contention policy is AGGRESSIVE: a conflicting
transaction just abort the competitor if the competitor is
not in COMMINTING status. A COMMINTING transaction can not
be aborted.(This is the so-called obstruction-free policy)

Also, we have a gentle contention policy POLITE: a conflicting
transaction halt a little second before tring to acquire that
object again.

For example, assume that transaction X already owns O, when
transaction Y want to access O. Then after transaction Y
made a TM-READ or TM-WRITE call, the underlying system will
abort transaction X if X is not in COMMINTING status.

Since every object has a pointer points to its owner transactionin
in its ownership record, no overheads exist when a transaction
is commiting.

#FUTURE READING:

[Herlihy 93]
Transactional Memory: Architectural Support for Lock-free Data Structures.

[Shavit 95]
Software Transactional Memory

[Harris 03]
Language Support for Lightweight Transactions

[Herlihy 03]
Software Transactional Memory for Dynamic-Sized Data Structures
