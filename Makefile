CPPFLAGS := -DTM_DEBUG
all:
	gcc $(CPPFLAGS) -O2 stm.c

clean:
	rm -f *.o a.out
