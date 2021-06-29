all: deliver.o packet.o helper.o
	g++ deliver.o packet.o helper.o -o deliver

deliver.o: deliver.c
	gcc -c deliver.c

packet.o: packet.c packet.h
	gcc -c packet.c

helper.o: helper.c helper.h
	gcc -c helper.c
	
clean:  
	rm *.o deliver 
