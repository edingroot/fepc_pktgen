all: gtp_generator udp_packet_sender echo_server
gtp_generator: gtp_generator.o 
	gcc -g -O2 -D_REENTRANT -Wall -Wno-unused-result -o gtp_generator gtp_generator.o
gtp_generator.o: gtp_generator.c
	gcc -g -O2 -D_REENTRANT -Wall -Wno-unused-result -c -o gtp_generator.o gtp_generator.c
udp_packet_sender:udp_packet_sender.o
	gcc -g -O2 -D_REENTRANT -Wall -o udp_packet_sender udp_packet_sender.o
udp_packet_sender.o: udp_packet_sender.c
	gcc -g -O2 -D_REENTRANT -Wall -c -o udp_packet_sender.o udp_packet_sender.c
echo_server: echo_server.o
	gcc -g -O2 -D_REENTRANT -Wall -o echo_server echo_server.o
echo_server.o: echo_server.c
	gcc -g -O2 -D_REENTRANT -Wall -c -o echo_server.o echo_server.c
clean:
	rm *.o gtp_generator udp_packet_sender echo_server
