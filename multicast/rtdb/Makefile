CC=gcc
CFLAGS= -Wall -O3 -g -fPIC

### for debug
# CFLAGS= -Wall -g
###

target=../lib/librtdb.so

all: $(target)
	make -C parser

rtdb_api.o: rtdb_api.c rtdb_api.h rtdb_user.h rtdb.h rtdb_comm.h rtdb_sem.c rtdb_sem.h

$(target): rtdb_api.o rtdb_sem.o
	gcc ${CFLAGS} -shared -o $@ $^

clean:
	rm -rf *.o $(target)
	make -C parser clean
