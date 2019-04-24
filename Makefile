INCLUDE_PATH=lib
CFLAGS=-O2 -D_REENTRANT -Wall

all: gtp_generator udp_packet_sender echo_server

udp_packet_sender: libudp.o
	gcc ${CFLAGS} -I${INCLUDE_PATH} -Wno-unused-result -o udp_packet_sender src/udp_packet_sender.c libudp.o

gtp_generator:
	gcc ${CFLAGS} -I${INCLUDE_PATH} -Wno-unused-result -o gtp_generator src/gtp_generator.c

echo_server:
	gcc ${CFLAGS} -I${INCLUDE_PATH} -o echo_server src/echo_server.c

# ------------------------------------------------------------------------------------- #
libcommon.o: lib/common.c
	gcc ${CFLAGS} -I${INCLUDE_PATH} -c -o libcommon.o lib/common.c

libudp.o: lib/udp.c
	gcc ${CFLAGS} -I${INCLUDE_PATH} -c -o libudp.o lib/udp.c
# ------------------------------------------------------------------------------------- #

clean:
	rm -f *.o gtp_generator udp_packet_sender echo_server
