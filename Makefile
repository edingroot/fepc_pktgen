all: gtp_generator udp_packet_sender echo_server

gtp_generator:
	gcc -g -O2 -D_REENTRANT -Wall -Wno-unused-result -c -o gtp_generator src/gtp_generator.c

udp_packet_sender:
	gcc -g -O2 -D_REENTRANT -Wall -c -o udp_packet_sender src/udp_packet_sender.c

echo_server:
	gcc -g -O2 -D_REENTRANT -Wall -c -o echo_server src/echo_server.c

clean:
	rm -f gtp_generator udp_packet_sender echo_server
