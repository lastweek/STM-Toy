CPPFLAGS := -DTM_DEBUG
SRCS = stm.c tls.c

all:
	gcc -c $(SRCS)

clean:
	rm -f *.o a.out
