all: server.o packet.o helper.o
	g++ server.o packet.o helper.o -o server

server.o: server.c
	gcc -c server.c

packet.o: packet.c packet.h
	gcc -c packet.c

helper.o: helper.c helper.h
	gcc -c helper.c
	
clean:  
	rm *.o server 
