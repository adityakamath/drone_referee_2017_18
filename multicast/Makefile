
HEADERS=rtdb/rtdb_user.h rtdb/rtdb_api.h rtdb/rtdbdefs.h rtdb/rtdb_sem.h
LIBS=lib/librtdb.so
BINS=bin/comm bin/listen bin/scomm
INSTALL=/usr

all: libs config/rtdb.ini

libs:
	make -C rtdb all
	make -C comm all

config/rtdb.ini: worldmodel/*.h config/rtdb.conf
	cd bin; ./xrtdb


clean:
	make -C rtdb clean
	make -C comm clean

install: all
	mkdir -p ${INSTALL}/include/rtdb
	mkdir -p ${INSTALL}/config
	cp  config/rtdb.ini ${INSTALL}/config
	cp  config/network.conf ${INSTALL}/config
	cp  ${HEADERS} ${INSTALL}/include/rtdb
	cp  ${LIBS}    ${INSTALL}/lib
	cp  ${BINS}    ${INSTALL}/bin

uninstall:
	rm -fv ${INSTALL}/include/${HEADERS}
	rm -fv ${INSTALL}/lib/${LIBS}
	rm -fv ${INSTALL}/bin/${BINS}
