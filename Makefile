CPPFLAGS := -DTM_DEBUG
SRCS = stm.c tls.c

all:
	gcc -g -O2 $(CPPFLAGS) $(SRCS)

clean:
	rm -f *.o a.out
	rm -f -r a.out.dSYM
