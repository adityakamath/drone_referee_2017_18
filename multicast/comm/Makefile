cbin=comm
sbin=scomm
lbin=listen
bin_dir=../bin
ctarget=$(bin_dir)/$(cbin)
ltarget=$(bin_dir)/$(lbin)
starget=$(bin_dir)/$(sbin)

INCLUDE = ../include

CC = gcc
CFLAGS = -g -O3 -Wall -I$(INCLUDE) -I../rtdb 
LDFLAGS=-L../lib -Wl,-rpath ../lib -lrt

all: $(ctarget) $(ltarget) $(starget)

multicast.o: multicast.h multicast.c

network_config.o: network_config.c network_config.h ../rtdb/rtdb_network.h

comm.c: ../rtdb/rtdb_user.h ../rtdb/rtdb_api.h

$(ctarget): multicast.o network_config.o $(cbin).o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lpthread -lrtdb -lz

$(ltarget): multicast.o network_config.o $(lbin).o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lpthread -lrtdb -lz

$(starget): $(sbin).o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lpthread -lrtdb -lz

clean:
	rm -rf *.o $(ctarget) $(ltarget) $(starget)
