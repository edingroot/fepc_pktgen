all: gtp_generator udp_packet_sender echo_server

INCLUDE_PATH=lib
CFLAGS=-O2 -D_REENTRANT -Wall -c

gtp_generator:
	gcc ${CFLAGS} -I${INCLUDE_PATH} -Wno-unused-result -o gtp_generator src/gtp_generator.c

udp_packet_sender:
	gcc ${CFLAGS} -I${INCLUDE_PATH} -o udp_packet_sender src/udp_packet_sender.c

echo_server:
	gcc ${CFLAGS} -I${INCLUDE_PATH} -o echo_server src/echo_server.c

clean:
	rm -f gtp_generator udp_packet_sender echo_server
